#include "networking.h"

INTERNAL void send_packet_no_lock(Server* server, Socket* client, std::string message);

INTERNAL
void handle_accept(Server* server, Socket* client) {
	server->mutex.lock();
	server->clients.push_back(client);
	BMT_LOG(INFO, "A new client has connected! %d total clients", server->clients.size());
	server->mutex.unlock();
	Socket* clientNew = new Socket(server->service);
	server->acceptor.async_accept(*clientNew, boost::bind(handle_accept, server, clientNew));
	boost::this_thread::sleep(boost::posix_time::millisec(LONG_SLEEP));
}

INTERNAL
void run_service(Server* server) {
	server->service.run();
}

INTERNAL
void stop_service(Server* server) {
	server->service.stop();
}

INTERNAL
void disconnect_client(Server* server, Socket* client, u16 index) {
	client->shutdown(boost::asio::ip::PROTOCOL::socket::shutdown_both);
	client->close();
	server->clients.erase(server->clients.begin() + index);

	server->userListMutex.lock();
	server->users.erase(server->users.begin() + index);
	server->userListMutex.unlock();

	delete client;

	BMT_LOG(INFO, "Client has disconnected! %d total clients", server->clients.size());
}

INTERNAL
void handle_new_connection(Server* server, Socket* socket, std::string name, std::string pass) {
	Account account;
	LoginState success = login(&account, name, pass);
	account.socket = socket;

	if (success == LOGIN_SUCCESS || success == LOGIN_CREATED) {
		//send names of all other users already connected to the new client.
		server->userListMutex.lock();
		for (u16 j = 0; j < server->users.size(); ++j) {
			std::string command = "name|";
			command.append(server->users[j].name);
			command.append("\n");
			account.socket->write_some(boost::asio::buffer(command, command.size()));
			boost::this_thread::sleep(boost::posix_time::millisec(LONG_SLEEP));
		}
		server->users.push_back(account);
		server->userListMutex.unlock();

		if (success == LOGIN_SUCCESS) {
			std::string command = format_text("login_success|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d\n",
				account.name.c_str(), account.pass.c_str(), account.standsheet.name.c_str(), account.standsheet.standTypes.c_str(),
				account.standsheet.standAbilityDesc.c_str(), account.standsheet.speed, account.standsheet.power, account.standsheet.range,
				account.standsheet.precision, account.standsheet.durability, account.standsheet.learning, account.usersheet.name.c_str(),
				account.usersheet.playername.c_str(), account.usersheet.gender.c_str(), account.usersheet.weight.c_str(), account.usersheet.height.c_str(),
				account.usersheet.bloodType.c_str(), account.usersheet.occupation.c_str(), account.usersheet.nationality.c_str(), account.usersheet.backstory.c_str(),
				account.usersheet.inventory.c_str(), account.usersheet.brains, account.usersheet.brawns, account.usersheet.bravery, account.usersheet.age,
				account.usersheet.totalHealth, account.usersheet.currentHealth, account.usersheet.resolveDamage, account.usersheet.bizarrePoints
			);
			send_packet_no_lock(server, socket, command);
		}
		else {
			std::string command = format_text("login_created|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d\n",
				account.name.c_str(), account.pass.c_str(), account.standsheet.name.c_str(), account.standsheet.standTypes.c_str(),
				account.standsheet.standAbilityDesc.c_str(), account.standsheet.speed, account.standsheet.power, account.standsheet.range,
				account.standsheet.precision, account.standsheet.durability, account.standsheet.learning, account.usersheet.name.c_str(),
				account.usersheet.playername.c_str(), account.usersheet.gender.c_str(), account.usersheet.weight.c_str(), account.usersheet.height.c_str(),
				account.usersheet.bloodType.c_str(), account.usersheet.occupation.c_str(), account.usersheet.nationality.c_str(), account.usersheet.backstory.c_str(),
				account.usersheet.inventory.c_str(), account.usersheet.brains, account.usersheet.brawns, account.usersheet.bravery, account.usersheet.age,
				account.usersheet.totalHealth, account.usersheet.currentHealth, account.usersheet.resolveDamage, account.usersheet.bizarrePoints
			);
			send_packet_no_lock(server, socket, command);
		}
	}
	if(success == LOGIN_FAILURE) {
		account.name = "Attempting connection...";
		server->userListMutex.lock();
		server->users.push_back(account);
		server->userListMutex.unlock();

		send_packet_no_lock(server, socket, "login_failure\n");
	}
}

INTERNAL
void read_handler(const boost::system::error_code) {

}

INTERNAL
void receive_loop(Server* server) {
	for (;;) {
		if (server->close) break;

		server->mutex.lock();
		for (u16 i = 0; i < server->clients.size(); ++i) {
			Socket* client = server->clients[i];

			if (client->available()) {
				char readBuffer[BUFFER_SIZE] = { 0 };
				boost::system::error_code error;
				//TODO(Corbin): check if client->read_some is thread safe or not. (or actually just make this async)
				server->mutex.unlock();
				int bytesRead = client->read_some(boost::asio::buffer(readBuffer, BUFFER_SIZE), error);
				server->mutex.lock();

				//check for if should disconnect
				if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error) || (boost::asio::error::connection_aborted == error)) {
					disconnect_client(server, client, i);
					break;
				}
				std::string msg(readBuffer, bytesRead);
				BMT_LOG(DEBUG, "Received instruction from client %d: %s", i, msg.c_str());
				if (msg == "exit") {
					disconnect_client(server, client, i);
					break;
				}

				//split string
				StringList commands = split_string(msg, '\n');
				for (u16 i = 0; i < commands.size(); ++i) {
					StringList tokens = split_string(msg, '|');
					//handle new connection (new clients send their name immediately after connecting)
					if (tokens.size() > 0) {
						if (tokens[0] == "name") {
							BMT_LOG(INFO, "User '%s' is attempting to connect with hashed password '%s'...", tokens[1].c_str(), tokens[3].c_str());
							handle_new_connection(server, client, tokens[1], tokens[3]);
						}
					}
					server->mutex.unlock();
					server->receiveCallback(server, client, &tokens);
					server->mutex.lock();

					//put received command into a queue to be sent back to all clients
					ClientMessage cm;
					cm.socket = client;
					cm.str = msg;
					server->messageQueue.push(cm);
				}
			}
		}
		server->mutex.unlock();
		//boost::this_thread::sleep(boost::posix_time::millisec(LONG_SLEEP));
	}
	BMT_LOG(INFO, "Closed receive_loop");
}

INTERNAL
void response_loop(Server* server) {
	for (;;) {
		if (server->close) break;

		if (!server->messageQueue.empty()) {
			ClientMessage msg = server->messageQueue.front();
			server->mutex.lock();
			for (u16 i = 0; i < server->clients.size(); ++i) {
				if (server->clients[i] != msg.socket)
					server->clients[i]->write_some(boost::asio::buffer(msg.str, msg.str.size()));
			}
			server->messageQueue.pop();
			server->mutex.unlock();
		}
		boost::this_thread::sleep(boost::posix_time::millisec(LONG_SLEEP));
	}
	BMT_LOG(INFO, "Closed response_loop");
}

Server::Server() : service(), acceptor( service, boost::asio::ip::PROTOCOL::endpoint(boost::asio::ip::PROTOCOL::v4(), 8001) ) {}

void start_server(Server* server, u32 port) {
	server->close = false;

	Socket* client = new Socket(server->service);
	server->acceptor.async_accept(*client, boost::bind(handle_accept, server, client));
	boost::this_thread::sleep(boost::posix_time::millisec(SHORT_SLEEP));

	server->threads.create_thread(boost::bind(run_service, server));
	boost::this_thread::sleep(boost::posix_time::millisec(SHORT_SLEEP));

	server->threads.create_thread(boost::bind(receive_loop, server));
	boost::this_thread::sleep(boost::posix_time::millisec(SHORT_SLEEP));

	server->threads.create_thread(boost::bind(response_loop, server));
	boost::this_thread::sleep(boost::posix_time::millisec(SHORT_SLEEP));
}

void stop_server(Server* server) {
	BMT_LOG(INFO, "------------------------------- Stopping server -------------------------------");
	server->close = true;
	server->mutex.lock();
	server->service.stop();
	server->acceptor.cancel();
	BMT_LOG(INFO, "joining threads...");
	server->threads.join_all();
	BMT_LOG(INFO, "threads joined");
	BMT_LOG(INFO, "-------------------------------- Stopped server -------------------------------");
	server->mutex.unlock();
}

void set_receive_callback(Server* server, void(*callback)(Server*, Socket*, StringList*)) {
	server->receiveCallback = callback;
}
 
INTERNAL
void packet_sent_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {

}

void send_packet(Server* server, Socket* client, std::string message) {
	server->mutex.lock();
	client->write_some( boost::asio::buffer(message, message.size()) );
	//client->async_write_some(boost::asio::buffer(message, message.size()), packet_sent_handler);
	server->mutex.unlock();
}

//sends a message to all connected clients
void send_packet_all(Server* server, std::string message) {
	server->mutex.lock();
	ClientMessage cm;
	cm.socket = NULL;
	cm.str = message;
	server->messageQueue.push(cm);
	server->mutex.unlock();
}

INTERNAL
void send_packet_no_lock(Server* server, Socket* client, std::string message) {
	client->write_some( boost::asio::buffer(message, message.size()) );
}
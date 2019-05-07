#ifndef NETWORKING_H
#define NETWORKING_H

#include "../DnDShared/globals.h"
#include "accounts.h"

struct Server {
	Server();
	boost::mutex mutex;
	boost::mutex userListMutex;
	ClientList clients;
	std::vector<Account> users;
	MessageQueue messageQueue;
	boost::asio::io_service service;
	boost::asio::ip::PROTOCOL::acceptor acceptor;
	boost::thread_group threads;
	volatile bool close;

	void(*receiveCallback)(Server*, Socket*, StringList*);
};

void start_server(Server* server, u32 port = 8001);
void stop_server(Server* server);
void set_receive_callback(Server* server, void(*callback)(Server*, Socket*, StringList*));
void send_packet(Server* server, Socket* client, std::string message);
//sends a message to all connected clients
void send_packet_all(Server* server, std::string message);

#endif
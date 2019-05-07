#include "accounts.h"
#include <fstream>
#define ACCOUNTS_DIR "data/accounts.txt"

INTERNAL
void append_value(std::string* str, std::string toApp) {
	str->append(toApp);
	str->append("|");
}

INTERNAL inline
StringList read_file() {
	StringList strings;
	std::string line;
	std::ifstream infile;
	infile.open(ACCOUNTS_DIR);
	if (infile.is_open()) {
		while (!infile.eof()) {
			getline(infile, line);
			strings.push_back(line);
		}
	}
	infile.close();
	return strings;
}

void write_account_data(Account* acc) {
	StringList lines = read_file();
	for (i32 i = 0; i < lines.size(); ++i) {
		StringList tokens = split_string(lines[i], '|');
		if (tokens.size() == 0)
			continue;

		if (tokens[0] == acc->name) {
			std::string* curr = &lines[i];
			curr->clear();
			append_value(curr, acc->name);
			append_value(curr, acc->pass);

			append_value(curr, acc->standsheet.name);
			append_value(curr, acc->standsheet.standTypes);
			append_value(curr, acc->standsheet.standAbilityDesc);
			append_value(curr, std::to_string(acc->standsheet.speed));
			append_value(curr, std::to_string(acc->standsheet.power));
			append_value(curr, std::to_string(acc->standsheet.range));
			append_value(curr, std::to_string(acc->standsheet.precision));
			append_value(curr, std::to_string(acc->standsheet.durability));
			append_value(curr, std::to_string(acc->standsheet.learning));

			append_value(curr, acc->usersheet.name);
			append_value(curr, acc->usersheet.playername);
			append_value(curr, acc->usersheet.gender);
			append_value(curr, acc->usersheet.weight);
			append_value(curr, acc->usersheet.height);
			append_value(curr, acc->usersheet.bloodType);
			append_value(curr, acc->usersheet.occupation);
			append_value(curr, acc->usersheet.nationality);
			append_value(curr, acc->usersheet.backstory);
			append_value(curr, acc->usersheet.inventory);
			append_value(curr, std::to_string(acc->usersheet.brains));
			append_value(curr, std::to_string(acc->usersheet.brawns));
			append_value(curr, std::to_string(acc->usersheet.bravery));
			append_value(curr, std::to_string(acc->usersheet.age));
			append_value(curr, std::to_string(acc->usersheet.totalHealth));
			append_value(curr, std::to_string(acc->usersheet.currentHealth));
			append_value(curr, std::to_string(acc->usersheet.resolveDamage));
			append_value(curr, std::to_string(acc->usersheet.bizarrePoints));
			break;
		}
	}

	std::ofstream outfile;
	outfile.open(ACCOUNTS_DIR);
	if (outfile.is_open()) {
		for (i32 i = 0; i < lines.size(); ++i) {
			outfile << lines.at(i) << "\n";
		}
	}

	BMT_LOG(INFO, "Saved new account info for [%s]", acc->name.c_str());
}

void read_stand_charsheet(StandCharSheet* sheet, StringList* tokens) {
	sheet->name = tokens->at(2);
	sheet->standTypes = tokens->at(3);
	sheet->standAbilityDesc = tokens->at(4);
	sheet->speed = std::stoi(tokens->at(5));
	sheet->power = std::stoi(tokens->at(6));
	sheet->range = std::stoi(tokens->at(7));
	sheet->precision = std::stoi(tokens->at(8));
	sheet->durability = std::stoi(tokens->at(9));
	sheet->learning = std::stoi(tokens->at(10));
}

void read_user_charsheet(UserCharSheet* sheet, StringList* tokens) {
	sheet->name = tokens->at(11);
	sheet->playername = tokens->at(12);
	sheet->gender = tokens->at(13);
	sheet->weight = tokens->at(14);
	sheet->height = tokens->at(15);
	sheet->bloodType = tokens->at(16);
	sheet->occupation = tokens->at(17);
	sheet->nationality = tokens->at(18);
	sheet->backstory = tokens->at(19);
	sheet->inventory = tokens->at(20);
	sheet->brains = std::stoi(tokens->at(21));
	sheet->brawns = std::stoi(tokens->at(22));
	sheet->bravery = std::stoi(tokens->at(23));
	sheet->age = std::stoi(tokens->at(24));
	sheet->totalHealth = std::stoi(tokens->at(25));
	sheet->currentHealth = std::stoi(tokens->at(26));
	sheet->resolveDamage = std::stoi(tokens->at(27));
	sheet->bizarrePoints = std::stoi(tokens->at(28));
}

LoginState login(Account* result, std::string username, std::string pass) {
	result->name = username;
	result->pass = pass;
	
	LoginState successful = LOGIN_FAILURE;

	//check if account exists
	bool doesAccountExist = false;
	std::string line;
	std::ifstream infile;
	infile.open(ACCOUNTS_DIR);
	if (infile.is_open()) {
		while (!infile.eof()) {
			getline(infile, line);

			StringList tokens = split_string(line, '|');
			if (tokens.size() == 0) continue;

			if (tokens.at(0) == username) {
				doesAccountExist = true;
				BMT_LOG(INFO, "Account [%s] exists.", username.c_str());
				
				if (tokens.at(1) == pass) {
					BMT_LOG(INFO, "Account password matches! [%s]", pass.c_str());
					successful = LOGIN_SUCCESS;

					//TODO(Corbin): now read all the other data into the account.
					read_stand_charsheet(&result->standsheet, &tokens);
					read_user_charsheet(&result->usersheet,   &tokens);
				}
				else {
					BMT_LOG(WARNING, "Password is incorrect!");
					*result = { 0 };
				}
				break;
			}
		}
	}
	infile.close();

	//create a new account
	if (doesAccountExist == false) {
		std::ofstream outfile;
		outfile.open(ACCOUNTS_DIR, std::ios_base::app);
		if (outfile.is_open()) {
			//Write blank data for the new account.
			outfile << username << "|" << pass << "|" <<
				/* Stand Char Sheet */
				"enter a name|stand type|stand ability description|0|0|0|0|0|0|" <<
				/* User Char Sheet  */
				"enter a name|player name|gender|weight|height|bloodType|occupation|nationality|backstory|inventory|0|0|0|0|0|0|0|0" <<
				"\n";

			BMT_LOG(INFO, "Created account for [%s]", username.c_str());
		}
		successful = LOGIN_CREATED;
	}

	return successful;
}
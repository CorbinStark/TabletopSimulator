#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <string>
#include "../DnDShared/defines.h"
#include "../DnDShared/globals.h"

struct StandCharSheet {
	std::string name;
	std::string standTypes;
	std::string standAbilityDesc;
	i32 speed;
	i32 power;
	i32 range;
	i32 precision;
	i32 durability;
	i32 learning;
};

struct UserCharSheet {
	std::string name;
	std::string playername;
	std::string gender;
	std::string weight;
	std::string height;
	std::string bloodType;
	std::string occupation;
	std::string nationality;
	std::string backstory;
	std::string inventory;
	i32 brains;  //NOTE(Corbin): valid values 0-5
	i32 brawns;   //NOTE(Corbin): valid values 0-5
	i32 bravery; //NOTE(Corbin): valid values 0-5
	i32 age;
	i32 totalHealth;
	i32 currentHealth;
	i32 resolveDamage;
	i32 bizarrePoints;
};

struct Account {
	Socket* socket;
	std::string name;
	std::string pass;
	//TODO(Corbin): allow multiple character sheets per account.
	StandCharSheet standsheet;
	UserCharSheet usersheet;
};

enum LoginState {
	LOGIN_SUCCESS,
	LOGIN_FAILURE,
	LOGIN_CREATED
};

void read_stand_charsheet(StandCharSheet* sheet, StringList* tokens);
void read_user_charsheet(UserCharSheet* sheet, StringList* tokens);
void write_account_data(Account* acc);
LoginState login(Account* result, std::string username, std::string pass);

#endif
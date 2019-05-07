#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <string>
#include "defines.h"
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

INTERNAL
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

INTERNAL
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

INTERNAL inline 
void load_account(Account* account, StringList* tokens) {
	tokens->erase(tokens->begin());
	read_stand_charsheet(&account->standsheet, tokens);
	read_user_charsheet(&account->usersheet, tokens);
}

#endif
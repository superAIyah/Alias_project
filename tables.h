#ifndef TASKMANAGER_DB_TABLES_H
#define TASKMANAGER_DB_TABLES_H

#include <iostream>

class User {
public:
    int id;
    std::string login;
    std::string username;
    std::string password;
    unsigned int num_of_wins = 0;
    unsigned int num_of_losses = 0;
    int points = 0;

//login - unique (системное)
    User(int id_, std::string login_, std::string username_ = "", std::string password_ = "") : id(id_), login(login_), username(username_), password(password_) {}

//login - unique
    User(std::string login_, std::string username_ = "", std::string password_ = "") : id(0), login(login_), username(username_), password(password_) {}
};

class Word {
public:
    std::string word;
    std::string stem;
    int level;

    Word(std::string new_word, std::string new_stem, int new_level=0) : word(new_word), stem(new_stem), level(new_level) {}
	Word() = default;
	Word& operator=(const Word& other) = default;
	Word(const Word& other) = default;
	~Word() = default;
};

#endif //TASKMANAGER_DB_TABLES_H

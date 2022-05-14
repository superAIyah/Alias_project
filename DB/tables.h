#ifndef TASKMANAGER_DB_TABLES_H
#define TASKMANAGER_DB_TABLES_H

#include <iostream>

//class User {
//public:
//    int id;
//    std::string login;
//    std::string username;
//    std::string password;
//
////login - unique (системное)
//    User(int id_, std::string login_, std::string username_ = "", std::string password_ = "") : id(id_), login(login_), username(username_), password(password_) {}
//
////login - unique
//    User(std::string login_, std::string username_ = "", std::string password_ = "") : id(0), login(login_), username(username_), password(password_) {}
//};
//
//class Task {
//public:
//    int id;
//    std::string head;
//    std::string body;
//    bool completion = false;
//    int assigner_id;
//    int executor_id;
//
////(системное)
//    Task(int id_, std::string head_, std::string body_, bool completion_, int assigner_id_, int executor_id_)
//            : id(id_), head(head_), body(body_), completion(completion_), assigner_id(assigner_id_), executor_id(executor_id_) {}
//
//    Task(std::string head_, std::string body_, int assigner_id_, int executor_id_)
//            : id(0), head(head_), body(body_), assigner_id(assigner_id_), executor_id(executor_id_) {}
//
////executor - 1
//    Task(std::string head_, std::string body_, int assigner_id_)
//            : id(0), head(head_), body(body_), assigner_id(assigner_id_), executor_id(/*assigner_id_*/1) {}
//
//    Task(std::string head_, int assigner_id_, int executor_id_)
//            : id(0), head(head_), assigner_id(assigner_id_), executor_id(executor_id_) {}
//};
//
//class Message {
//public:
////(системное)
//    Message(int id_, std::string text_, int task_id_, int from_id_)
//            : id(id_), text(text_), task_id(task_id_), from_id(from_id_) {}
//
//    Message(std::string text_, int task_id_, int from_id_)
//            : id(0), text(text_), task_id(task_id_), from_id(from_id_) {}
//
//    int id;
//    std::string text;
//    int task_id;
//    int from_id;
//};

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
};

#endif //TASKMANAGER_DB_TABLES_H

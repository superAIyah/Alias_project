#include <iostream>
#include <cassert>
#include <memory>
#include "DBConnection.h"
#include "UserDBManager.h"
#include "WordDBManager.h"

const std::string HOST = "localhost";
const std::string SCHEMA_NAME = "test";
const std::string USERNAME1 = "server_1";
const std::string PWD1 = "1111";
const std::string USERNAME2 = "server_2";
const std::string PWD2 = "2222";


void premade(IUserDBManager& user_manager, IWordDBManager& word_manager, ConnectionProxy& proxy){
    assert(proxy.valid());
    std::cout<<"Connection valid\n";
    assert(proxy->IsHealthy());
    std::cout<<"Connection healthy\n";
    user_manager.add_user(User("AAA"));
    user_manager.add_user(User("BBB"));
    user_manager.add_user(User("CCC"));
    User B = user_manager.search_user("BBB");
    assert(B.login == user_manager.get_user(B.id).login);
    std::cout<<"1 done\n";
    user_manager.add_user(User("DDD"));
    std::vector<int> ids = {1, 2, 4};
    std::vector<User> users = user_manager.get_all_users(ids);
    assert(users[0].login == "AAA");
    std::cout<<"2 done\n";
    assert(users[1].login == "BBB");
    std::cout<<"3 done\n";
    assert(users[2].login == "DDD");
    std::cout<<"4 done\n";

	UserInfo tmp = user_manager.GetInfo("CCC");
	user_manager.UpdateUser("BBB", false);


//    word_manager.add_word(Word("яблоко", "яблок", 1));
//    word_manager.add_word(Word("мышь", "мыш", 1));
//    word_manager.add_word(Word("табуретка", "табурет", 2));
//    word_manager.add_word(Word("капельница", "кап", 2));
//    std::vector<Word> words = word_manager.get_words(1, 2);
//    assert((words[0].word == "яблоко" && words[1].stem == "мыш") || (words[1].word == "яблоко" && words[0].stem == "мыш"));
//    std::cout<<"5 done\n";
//    words = word_manager.get_words(2, 2);
//    assert((words[0].word == "табуретка" && words[1].stem == "кап") || (words[1].word == "табуретка" && words[0].stem == "кап"));
//    std::cout<<"6 done\n";
}

class testclass{
public:
    testclass(ConnectionProxy& ConnProxy){
        user_manager = new UserDBManager(*ConnProxy);
        word_manager = new WordDBManager(*ConnProxy);
    }

    IUserDBManager* user_manager;
    IWordDBManager* word_manager;
};

int main() {
//    DBConnection connection(HOST, USERNAME1, PWD1, SCHEMA_NAME);
//    std::unique_ptr<DBConnection> tmp = std::make_unique<DBConnection>(HOST, USERNAME1, PWD1, SCHEMA_NAME);
    std::vector<std::unique_ptr<DBConnection>> connections;
    connections.push_back(std::make_unique<DBConnection>(HOST, USERNAME1, PWD1, SCHEMA_NAME));
    connections.push_back(std::make_unique<DBConnection>(HOST, USERNAME2, PWD2, SCHEMA_NAME));
    std::unique_ptr<IConnectionPool> ConnPool = std::make_unique<ConnectionPool>();
    ConnPool->FillPool(std::move(connections));
    ConnectionProxy ConnProxy = ConnPool->get_connection();

//    UserDBManager user_manager(connection.session);
//    WordDBManager word_manager(connection.session);

//    IUserDBManager* user_manager = new UserDBManager(*ConnProxy);
//    IWordDBManager* word_manager = new WordDBManager(*ConnProxy);

    testclass a(ConnProxy);


    premade(*a.user_manager, *a.word_manager, ConnProxy);

//    user_manager->drop();
//    word_manager->drop();

}

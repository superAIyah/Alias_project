#include "UserDBManager.h"

//UserDBManager::~UserDBManager() noexcept {
//    session.close();
//}

void UserDBManager::UseConnection(DBConnection & conn) {
    session = &conn.session;
    schema = &conn.schema;
//    user = schema->getTable("users", true);
    user = schema->getTable("users", true);
}

bool UserDBManager::add_user(const User& user_) {
    try{
        user.insert("login", "username", "password").values(user_.login, user_.username, user_.password).execute();
    } catch (mysqlx::abi2::r0::Error) {
        return false;
    }
    return true;
}

User UserDBManager::get_user(int id) {
    std::string res = user.select("login").where("id = :id").bind("id", id).execute().fetchOne()[0].get<std::string>();
    return User(id, res);
}

User UserDBManager::search_user(std::string login_) {
    mysqlx::Row row = user.select("id", "login").where("login = :login").bind("login", login_).execute().fetchOne();
    if(!row) throw DB_except("No user with this login!");
    int id = row[0].get<int>();
    std::string name = row[1].get<std::string>();
    return User(id, name);
}

std::vector<User> UserDBManager::get_all_users(std::vector<int> UserIDs){
    std::vector<User> users;
    for(int i=0; i<UserIDs.size(); ++i){
        mysqlx::Row row = user.select("id", "login").where("id = :ids").bind("ids", UserIDs[i]).execute().fetchOne();
        User tmp(row[0].get<int>(), row[1].get<std::string>());
        users.push_back(tmp);
    }
    return users;
}

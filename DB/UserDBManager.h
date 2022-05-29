#pragma once

#include "DBConnection.h"
#include "IUserDBManager.h"

class UserDBManager : public IUserDBManager {
private:
    mysqlx::Session* session;
    mysqlx::Schema* schema;
    mysqlx::Table user;
public:
    void UseConnection(DBConnection&) override;

    UserDBManager(DBConnection& conn) : session(&conn.session), schema(&conn.schema),
                                               user(schema->getTable("users", true)) {}
//    UserDBManager() = delete;

    bool add_user(const User &user) override;

    User get_user(int id) override;

    User search_user(std::string login_) override;

    std::vector<User> get_all_users(std::vector<int> UserIDs) override;

	bool has_user(std::string login_) override;

	UserInfo GetInfo(std::string login_) override;

	void UpdateUser(std::string login, bool win) override;

    void drop() override { session->sql("DROP TABLE users;").execute(); }
};


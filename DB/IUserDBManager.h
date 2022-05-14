#pragma once

#include <vector>
#include "tables.h"

//интерфейс для менеджера пользователей в БД
class IUserDBManager {
public:
    virtual void UseConnection(DBConnection&) = 0;

//добавление пользователя в БД
    virtual bool add_user(const User &user) = 0;

//получение пользователя по id
    virtual User get_user(int id) = 0;

//получение пользователя по логину
    virtual User search_user(std::string login_) = 0;

//получение всех пользователей
    virtual std::vector<User> get_all_users(std::vector<int> UserIDs) = 0;

//сброс таблицы (системное)
    virtual void drop() = 0;
};

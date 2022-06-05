#pragma once

#include <vector>
#include "tables.h"

//интерфейс для менеджера пользователей в БД
class IWordDBManager {
public:
    virtual void UseConnection(DBConnection&) = 0;

//добавление пользователя в БД
    virtual bool add_word(const Word &word_) = 0;

//получение пользователя по id
    virtual std::vector<Word> get_words(int lvl, int num) = 0;

    virtual std::vector<std::string> get_words_str(int lvl, int num) = 0;

//сброс таблицы (системное)
    virtual void drop() = 0;
};

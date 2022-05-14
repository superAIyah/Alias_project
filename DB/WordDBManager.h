#pragma once

#include "DBConnection.h"
#include "IWordDBManager.h"

class WordDBManager : public IWordDBManager {
private:
    mysqlx::Session* session;
    mysqlx::Schema* schema;
    mysqlx::Table word;
public:
    void UseConnection(DBConnection&) override;

    WordDBManager(DBConnection& conn) : session(&conn.session), schema(&conn.schema),
                                        word(schema->getTable("words", true)) {}

    bool add_word(const Word &word_) override;

    std::vector<Word> get_words(int lvl, int num) override;

    std::vector<std::string> get_words_str(int lvl, int num) override;

    void drop() override { session->sql("DROP TABLE words;").execute(); }
};


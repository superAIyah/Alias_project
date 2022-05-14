#include "WordDBManager.h"


void WordDBManager::UseConnection(DBConnection & conn) {
    session = &conn.session;
    schema = &conn.schema;
//    user = schema->getTable("users", true);
    word = schema->getTable("words", true);
}

bool WordDBManager::add_word(const Word &word_) {
    try {
        word.insert("word", "stem", "level").values(word_.word, word_.stem, word_.level).execute();
    } catch (mysqlx::abi2::r0::Error) {
        return false;
    }
    return true;
}

std::vector<Word> WordDBManager::get_words(int lvl, int num) {
    std::vector<Word> words;
//    mysqlx::RowResult res = word.select("word", "stem").where("level = :lvl").groupBy("RAND()").limit(num).bind("lvl", lvl).execute();
    mysqlx::RowResult res = session->sql("SELECT word, stem FROM words WHERE level = ? LIMIT ?;").bind(lvl).bind(num).execute();
    while (mysqlx::Row row = res.fetchOne()) {
        Word tmp(row[0].get<std::string>(), row[1].get<std::string>());
        words.push_back(tmp);
    }
    return words;
}

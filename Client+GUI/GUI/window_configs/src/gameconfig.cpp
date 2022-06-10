#include "gameconfig.h"

GameConfig::GameConfig(std::string sep /*= '|'*/) : separator(sep), size(0) {}

void GameConfig::AddSetting(std::string new_setting) {
//    if (size == 0) {
//        settings += new_setting;
//    }
//    else {
//        settings += separator;
//        settings += new_setting;
//    }
    settings += new_setting;
    settings += separator;
    if (new_setting != "")
        size++;
}

std::string GameConfig::GetSettings() {
    return settings;
}


#include "gameconfig.h"

#include <utility>

GameConfig::GameConfig(std::string sep /*= '|'*/) : separator(std::move(sep)), size(0) {}

void GameConfig::AddSetting(const std::string& new_setting) {
//    if (size == 0) {
//        settings += new_setting;
//    }
//    else {
//        settings += separator;
//        settings += new_setting;
//    }
    settings += new_setting;
    settings += separator;
    if (!new_setting.empty())
        size++;
}

std::string GameConfig::GetSettings() {
    return settings;
}


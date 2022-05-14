#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <iostream>

class GameConfig {
public:
    GameConfig(char sep = '|');
    void AddSetting(std::string);
    std::string GetSettings();
private:
    char separator;
    int size;
    std::string settings;

};

#endif // GAMECONFIG_H

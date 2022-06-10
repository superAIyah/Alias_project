#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <iostream>
#include <string>

class GameConfig {
public:
    GameConfig(std::string sep = "\r\n");
    void AddSetting(std::string);
    std::string GetSettings();
    int size;
private:
    std::string separator;
    std::string settings;

};

#endif // GAMECONFIG_H

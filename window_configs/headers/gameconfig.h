#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <iostream>
#include <string>

class GameConfig {
public:
    GameConfig(std::string sep = "\r\n");
    void AddSetting(std::string);
    std::string GetSettings();
private:
    std::string separator;
    int size;
    std::string settings;

};

#endif // GAMECONFIG_H

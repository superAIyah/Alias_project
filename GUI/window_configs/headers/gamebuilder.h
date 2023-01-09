#ifndef GAMEBUILDER_H
#define GAMEBUILDER_H

#include <iostream>
#include "Setting.h"
#include "gameconfig.h"

class GameBuilder {
public:
    explicit GameBuilder(std::vector<ISetting *>);
    GameConfig CollectSettings();
private:
    std::vector<ISetting *> settings;
};

#endif // GAMEBUILDER_H

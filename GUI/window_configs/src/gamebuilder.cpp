#include "gamebuilder.h"

#include <utility>

GameBuilder::GameBuilder(std::vector<ISetting *> configs) : settings(std::move(configs))
{ }

GameConfig GameBuilder::CollectSettings() {
    GameConfig game_conf;
    for (auto setting : settings) {
        game_conf.AddSetting(setting->collectSetting());
    }
    return game_conf;
}

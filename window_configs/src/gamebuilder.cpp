#include "gamebuilder.h"

GameBuilder::GameBuilder(std::vector<ISetting *> configs) : settings(configs)
{ }

GameConfig GameBuilder::CollectSettings() {
    GameConfig game_conf;
    for (int i = 0; i < settings.size(); i++) {
        game_conf.AddSetting(settings[i]->collectSetting());
    }
    return game_conf;
}

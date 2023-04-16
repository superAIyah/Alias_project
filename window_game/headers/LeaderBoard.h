#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <vector>
#include "Leader.h"

class LeaderBoard {
public:
    LeaderBoard(std::vector<Leader>);
    std::vector<Leader> leaders;
    int size;
};

#endif // LEADERBOARD_H

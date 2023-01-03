#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <vector>
#include "Leader.h"

class LeaderBoard {
public:
    explicit LeaderBoard(std::vector<Leader>);
	LeaderBoard() = default;
	void Clear();
    std::vector<Leader> leaders;
    unsigned int size{};
};

#endif // LEADERBOARD_H

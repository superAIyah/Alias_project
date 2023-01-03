#include "LeaderBoard.h"

#include <utility>

LeaderBoard::LeaderBoard(std::vector<Leader> lrs)
{
    leaders = std::move(lrs);
    size = leaders.size();
}

void LeaderBoard::Clear(){
	leaders.clear();
	size = 0;
}

#include "LeaderBoard.h"

LeaderBoard::LeaderBoard(std::vector<Leader> lrs)
{
    leaders = lrs;
    size = leaders.size();
}

void LeaderBoard::Clear(){
	leaders.clear();
	size = 0;
}

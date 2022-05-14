#ifndef BOARD_H
#define BOARD_H

#include "ILeaderBoard.h"

class Board : public ILeaderBoard
{
public:
    Board();
    unsigned int UpdateLeaderboard(LeaderBoard lb) override;

};

#endif // BOARD_H

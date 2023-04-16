#ifndef ILEADERBOARD_H
#define ILEADERBOARD_H

#include "LeaderBoard.h"

class IBoard {
public:
    virtual unsigned int UpdateLeaderboard(LeaderBoard lb) = 0;
};

#endif // ILEADERBOARD_H

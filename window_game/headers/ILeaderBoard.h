#ifndef ILEADERBOARD_H
#define ILEADERBOARD_H

#include "LeaderBoard.h"

class ILeaderBoard {
public:
    virtual unsigned int UpdateLeaderboard(LeaderBoard lb) = 0;
};

#endif // ILEADERBOARD_H

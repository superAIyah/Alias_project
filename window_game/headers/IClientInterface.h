#ifndef ICLIENTINTERFACE_H
#define ICLIENTINTERFACE_H

#include "iTimerController.h"
#include "IMessenger.h"
#include "IBoard.h"

class IClientInterface
{
public:
     IBoard *board;
     ITimerController *timer;
     IMessenger *messenger;
};

#endif // ICLIENTINTERFACE_H

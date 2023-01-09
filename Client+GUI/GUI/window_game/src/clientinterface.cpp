#include "clientinterface.h"


ClientInterface::ClientInterface(Timer *tmr, Messenger *msgr, Board *brd) : IClientInterface()
{
    timer = tmr;
    messenger = msgr;
    board = brd;
}

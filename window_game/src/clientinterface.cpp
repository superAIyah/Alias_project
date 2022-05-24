#include "clientinterface.h"


ClientInterface::ClientInterface(Timer *tmr, Messenger *msgr, Board *brd)
{
    timer = tmr;
    messenger = msgr;
    board = brd;
}

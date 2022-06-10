#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

#include "IClientInterface.h"
#include "timer.h"
#include "messenger.h"
#include "board.h"

class ClientInterface : public IClientInterface
{
public:
    ClientInterface(Timer*, Messenger*, Board*);
};

#endif // CLIENTINTERFACE_H

#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include <vector>
#include "Message.h"

class ICommunication // интерфейс таймера
{
public:
    virtual unsigned int ShowMessages(std::vector<Message> messages) = 0;
    virtual unsigned int UpdateKeyword(std::string keyword) = 0;
};

#endif // ICOMMUNICATION_H

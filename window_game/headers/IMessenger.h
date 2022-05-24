#ifndef ICOMMUNICATION_H
#define ICOMMUNICATION_H

#include <vector>
#include "Message.h"

class IMessenger // интерфейс таймера
{
public:
    virtual unsigned int ShowMessages(const std::vector<Message>& messages) = 0;
    virtual unsigned int UpdateKeyword(const std::string& keyword) = 0;
};

#endif // ICOMMUNICATION_H

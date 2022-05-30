#include "Message.h"

Message::Message(std::string name, bool me, std::string msg)
    : name(name), me(me), msg(msg)
{ }

void Message::colorIt()
{
    return;
}

#include "Message.h"

Message::Message(int id, std::string name, bool me, std::string msg)
    : id(id), name(name), me(me), msg(msg)
{ }

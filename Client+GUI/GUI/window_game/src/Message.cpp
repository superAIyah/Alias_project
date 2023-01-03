#include "Message.h"

#include <utility>

Message::Message(std::string name, bool me, std::string msg)
    : name(std::move(name)), me(me), msg(std::move(msg))
{ }

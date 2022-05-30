#ifndef MESSAGE_H
#define MESSAGE_H

#include "Leader.h"
#include "LeaderBoard.h"

class Message{
public:
    Message(std::string, bool, std::string);
	Message() = default;
    void colorIt();

    std::string name; // имя пользователя
    bool me; // флаг, мое ли это сообщение (чтобы виделять отдельным цветом)
    std::string msg;
};

#endif // MESSAGE_H

#ifndef LEADER_H
#define LEADER_H

#include <iostream>

class Leader {
public:
    Leader(std::string, int, bool);
    std::string name;
    int points;
    bool host;
};

#endif // LEADER_H

#include "Leader.h"

#include <utility>

Leader::Leader(std::string name, int points, bool host)
    : name(std::move(name)), points(points), host(host)
{}

//
// Created by michael on 06.10.15.
//

#include <World/Space/SpaceLocation.h>

using namespace weave;
using namespace std;

const std::string weave::SpaceLocation::Type = "location";

std::string SpaceLocation::ToString() const noexcept {
    return "(" + to_string(X) + ", " + to_string(Y) + ", " + to_string(Z) + ")";
}

SpaceLocation::SpaceLocation(uint64_t id, int x, int y, int z) : WorldEntity(id), X(x), Y(y), Z(z) {
}

SpaceLocation::SpaceLocation(int x, int y, int z) : SpaceLocation(NoID, x, y, z) {
}

SpaceLocation::SpaceLocation() : SpaceLocation(0, 0, 0) {
}

std::string SpaceLocation::GetType() const noexcept {
    return Type;
}

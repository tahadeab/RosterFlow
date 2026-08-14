#include "roster.hpp"
#include <cassert>
#include <iostream>

int main() {
    RosterService service;
    std::string error;
    assert(service.addPlayer({10, "Test Player", 20, "Testland", "Midfielder", 2}, error));
    assert(!service.addPlayer({10, "Duplicate", 21, "Testland", "Forward", 3}, error));
    assert(service.findPlayer(10) != nullptr);
    assert(service.nextPriorityPlayer() != nullptr);
    assert(service.updatePriority(10, 1, error));
    assert(service.removePlayer(10, error));
    assert(service.findPlayer(10) == nullptr);
    assert(service.addPlayer({11, "Undo Player", 22, "Testland", "Defender", 1}, error));
    assert(service.undoLastAddition(error));
    assert(service.findPlayer(11) == nullptr);
    std::cout << "All RosterFlow tests passed.\n";
    return 0;
}

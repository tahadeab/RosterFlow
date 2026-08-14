#include "roster.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace rosterflow;

namespace {
Player player(int number, const std::string& name, int priority = 5) {
    return {number, name, 22, "Testland", "Midfielder", priority};
}

void testDoublyLinkedRoster() {
    DoublyLinkedRoster roster;
    assert(roster.empty());
    assert(roster.add(player(1, "One")));
    assert(roster.add(player(2, "Two")));
    assert(!roster.add(player(1, "Duplicate")));
    assert(roster.insertAt(player(3, "Inserted"), 1));
    assert(roster.size() == 3);
    const auto values = roster.toVector();
    assert(values[0].number == 1 && values[1].number == 3 && values[2].number == 2);
    assert(roster.findByNumber(3) != nullptr);
    assert(roster.findByNumber(99) == nullptr);
    assert(roster.removeByNumber(3));
    assert(!roster.removeByNumber(99));
    assert(roster.size() == 2);
    roster.clear();
    assert(roster.empty());
}

void testPriorityQueue() {
    PriorityQueue queue;
    assert(queue.enqueue(player(1, "Low", 5)));
    assert(queue.enqueue(player(2, "High", 1)));
    assert(queue.enqueue(player(3, "Medium", 3)));
    assert(!queue.enqueue(player(2, "Duplicate", 2)));
    assert(queue.peek() && queue.peek()->number == 2);
    assert(queue.updatePriority(1, 0));
    assert(queue.peek() && queue.peek()->number == 1);
    assert(!queue.updatePriority(99, 1));
    Player result;
    assert(queue.dequeue(result) && result.number == 1);
    assert(queue.dequeue(result) && result.number == 2);
    assert(queue.dequeue(result) && result.number == 3);
    assert(!queue.dequeue(result));
    assert(queue.empty());
}

void testActionStack() {
    ActionStack stack;
    assert(stack.empty());
    stack.push(player(1, "First"));
    stack.push(player(2, "Second"));
    assert(stack.peek() && stack.peek()->number == 2);
    Player result;
    assert(stack.pop(result) && result.number == 2);
    assert(stack.pop(result) && result.number == 1);
    assert(!stack.pop(result));
    assert(stack.empty());
}

void testServiceValidationAndOperations() {
    RosterService service;
    std::string error;
    assert(service.addPlayer(player(10, "Valid", 2), error));
    assert(!service.addPlayer(player(10, "Duplicate", 3), error));
    assert(!service.addPlayer({0, "Invalid Number", 20, "Testland", "Forward", 2}, error));
    assert(!service.addPlayer({11, "Invalid Age", 12, "Testland", "Forward", 2}, error));
    assert(!service.addPlayer({12, "Invalid Priority", 20, "Testland", "Forward", 0}, error));
    assert(service.findPlayer(10) != nullptr);
    assert(service.findPlayer(99) == nullptr);
    assert(service.updatePriority(10, 1, error));
    assert(service.nextPriorityPlayer() && service.nextPriorityPlayer()->priority == 1);
    assert(!service.updatePriority(10, 11, error));
    assert(!service.updatePriority(99, 2, error));
    assert(service.addPlayer(player(11, "Second", 3), error));
    assert(service.lastAddedPlayer() && service.lastAddedPlayer()->number == 11);
    assert(service.undoLastAddition(error));
    assert(service.findPlayer(11) == nullptr);
    assert(service.undoLastAddition(error));
    assert(service.findPlayer(10) == nullptr);
    assert(!service.undoLastAddition(error));
    assert(!service.removePlayer(10, error));
    assert(service.size() == 0);
}

void testPersistence() {
    const std::string path = "/tmp/rosterflow_unit_test.csv";
    std::remove(path.c_str());
    RosterService source;
    std::string error;
    assert(source.addPlayer(player(7, "Persisted One", 1), error));
    assert(source.addPlayer(player(8, "Persisted Two", 2), error));
    assert(source.saveCsv(path, error));

    RosterService restored;
    assert(restored.loadCsv(path, error));
    assert(restored.size() == 2);
    assert(restored.findPlayer(7) && restored.findPlayer(7)->name == "Persisted One");
    assert(restored.nextPriorityPlayer() && restored.nextPriorityPlayer()->number == 7);

    std::ofstream invalid(path);
    invalid << "number,name,age,nationality,position,priority\n";
    invalid << "not-a-number,Broken,20,Testland,Forward,1\n";
    invalid.close();
    assert(!restored.loadCsv(path, error));
    std::remove(path.c_str());
    assert(!restored.loadCsv(path, error));
}
}

int main() {
    testDoublyLinkedRoster();
    testPriorityQueue();
    testActionStack();
    testServiceValidationAndOperations();
    testPersistence();
    std::cout << "All RosterFlow unit tests passed.\n";
    return 0;
}

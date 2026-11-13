#include "roster.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace rosterflow;

namespace {

Player player(int number, const std::string& name, int priority = 5) {
    return {number, name, 22, "Testland", "Midfielder", priority};
}

// ---------------------------------------------------------------------------
// Existing behaviour must not regress.
// ---------------------------------------------------------------------------

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
    // Out-of-range priorities must be rejected by the queue itself, so the
    // invariant "every stored priority is valid" can never be broken.
    assert(!queue.updatePriority(1, 0));
    assert(!queue.updatePriority(1, 11));
    assert(queue.peek() && queue.peek()->number == 2);
    assert(queue.updatePriority(1, 1)); // valid in-range update; ties keep insertion order
    assert(queue.peek() && queue.peek()->number == 2);
    assert(!queue.updatePriority(3, 0)); // still rejected by the queue's own rule
    assert(!queue.updatePriority(99, 1));
    Player result;
    assert(queue.dequeue(result) && result.number == 2); // priority 1 (inserted first among ties)
    assert(queue.dequeue(result) && result.number == 1); // promoted to priority 1, tie -> later
    assert(queue.dequeue(result) && result.number == 3);
    assert(!queue.dequeue(result));
    assert(queue.empty());
}

void testPriorityQueueRemove() {
    PriorityQueue queue;
    queue.enqueue(player(1, "A", 2));
    queue.enqueue(player(2, "B", 1));
    queue.enqueue(player(3, "C", 3));
    assert(queue.removeByNumber(2));
    assert(!queue.removeByNumber(99));
    assert(queue.size() == 2);
    assert(queue.peek()->number == 1); // order preserved without full drain
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

// Regression: a rejected add must never desynchronise the roster and queue.
void testRejectedAddKeepsStructuresInSync() {
    RosterService service;
    std::string error;
    assert(service.addPlayer(player(5, "Keeper", 1), error));
    assert(!service.addPlayer(player(5, "Ghost", 2), error));
    assert(service.size() == 1);
    assert(service.priorityQueue().size() == 1);
    assert(service.integrityIssues().empty());
}

// ---------------------------------------------------------------------------
// New features.
// ---------------------------------------------------------------------------

void testCsvQuotingRoundTrip() {
    // Names containing commas previously corrupted the CSV file.
    Player tricky{7, "St. Clair, Junior", 24, "France, Metro", "Forward \"the one\"", 2};
    const std::string line = playerToCsv(tricky);
    const auto fields = splitCsvLine(line);
    assert(fields.size() == 6);
    assert(csvUnescape(fields[1]) == "St. Clair, Junior");
    assert(csvUnescape(fields[4]) == "Forward \"the one\"");

    const std::string path = "/tmp/rosterflow_quoting_test.csv";
    std::remove(path.c_str());
    RosterService source;
    std::string error;
    assert(source.addPlayer(tricky, error));
    assert(source.saveCsv(path, error));
    RosterService restored;
    assert(restored.loadCsv(path, error));
    assert(restored.size() == 1);
    const auto* loaded = restored.findPlayer(7);
    assert(loaded && loaded->name == "St. Clair, Junior");
    assert(loaded && loaded->position == "Forward \"the one\"");
    std::remove(path.c_str());
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
    // Regression: a corrupt file must not wipe the live roster.
    assert(restored.size() == 2);
    assert(restored.findPlayer(7) != nullptr);

    std::remove(path.c_str());
    assert(!restored.loadCsv(path, error));
}

void testSearchAndFilter() {
    RosterService service;
    std::string error;
    service.addPlayer({9, "Robert Lewandowski", 35, "Poland", "Forward", 4}, error);
    service.addPlayer({8, "Pedri", 21, "Spain", "Midfielder", 3}, error);
    service.addPlayer({11, "Raphinha", 27, "Brazil", "Forward", 5}, error);

    assert(service.search("forward").size() == 2);       // position, case-insensitive
    assert(service.search("PEDRI").size() == 1);          // name
    assert(service.search("spain").size() == 1);          // nationality
    assert(service.search("defender").empty());           // no match -> empty state
    assert(service.search("").empty());                   // blank query is safe
    assert(service.filterByPosition("Forward").size() == 2);
    assert(service.filterByPosition("Midfielder").size() == 1);
}

void testUpdatePlayer() {
    RosterService service;
    std::string error;
    assert(service.addPlayer(player(10, "Old Name", 2), error));
    Player edited = *service.findPlayer(10);
    edited.name = "New Name";
    edited.age = 30;
    edited.priority = 1;
    assert(service.updatePlayer(edited, error));
    const auto* stored = service.findPlayer(10);
    assert(stored && stored->name == "New Name" && stored->age == 30 && stored->priority == 1);
    assert(service.priorityQueue().front().number == 10);
    assert(service.size() == 1);
    assert(service.integrityIssues().empty());

    assert(!service.updatePlayer({99, "Ghost", 20, "Nowhere", "Forward", 1}, error));
    assert(!service.updatePlayer({10, "", 20, "Nowhere", "Forward", 1}, error)); // invalid name
    assert(service.findPlayer(10) && service.findPlayer(10)->name == "New Name"); // still intact
}

void testStatsAndReports() {
    std::vector<Player> players = {
        {1, "GK", 32, "Germany", "Goalkeeper", 2},
        {4, "DEF", 25, "Uruguay", "Defender", 1},
        {9, "FWD", 35, "Poland", "Forward", 4},
        {11, "FWD2", 27, "Brazil", "Forward", 5},
    };
    const auto stats = computeStats(players);
    assert(stats.totalPlayers == 4);
    assert(stats.byPosition.size() == 3);
    assert(stats.byPosition.front().position == "Forward" && stats.byPosition.front().count == 2);
    assert(stats.nationalities.size() == 4);
    assert(stats.youngestNumber == 4 && stats.oldestNumber == 9);

    const auto report = buildTextReport(players, "TEST-STAMP");
    assert(report.find("ROSTERFLOW SQUAD REPORT") != std::string::npos);
    assert(report.find("TEST-STAMP") != std::string::npos);
    assert(report.find("Forward: 2") != std::string::npos);

    const auto csv = buildCsvReport(players);
    assert(csv.find("number,name,age,nationality,position,priority") != std::string::npos);
    assert(csv.find("#1,") == std::string::npos); // plain numbers only, no decoration

    const auto emptyReport = buildTextReport({}, "TEST-STAMP");
    assert(emptyReport.find("roster is empty") != std::string::npos); // empty state handled
}

void testIntegrityCheck() {
    RosterService service;
    std::string error;
    service.addPlayer(player(1, "A"), error);
    service.addPlayer(player(2, "B"), error);
    assert(service.integrityIssues().empty());
}

void testValidationHelpers() {
    assert(isValidNumber(1));
    assert(!isValidNumber(0));
    assert(!isValidNumber(-3));
    assert(isValidName("Ok"));
    assert(!isValidName("   "));
    assert(isValidAge(rules::kMinAge));
    assert(isValidAge(rules::kMaxAge));
    assert(!isValidAge(rules::kMaxAge + 1));
    assert(isValidPriority(rules::kMinPriority));
    assert(!isValidPriority(rules::kMaxPriority + 1));
    assert(validatePlayer(player(1, "Fine")).empty());
    assert(!validatePlayer({-1, "Bad", 20, "X", "Y", 1}).empty());
}

} // namespace

int main() {
    testDoublyLinkedRoster();
    testPriorityQueue();
    testPriorityQueueRemove();
    testActionStack();
    testServiceValidationAndOperations();
    testRejectedAddKeepsStructuresInSync();
    testCsvQuotingRoundTrip();
    testPersistence();
    testSearchAndFilter();
    testUpdatePlayer();
    testStatsAndReports();
    testIntegrityCheck();
    testValidationHelpers();
    std::cout << "All RosterFlow unit tests passed.\n";
    return 0;
}

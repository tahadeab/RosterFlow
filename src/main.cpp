#include "roster.hpp"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

using namespace rosterflow;

namespace {
void line() { std::cout << "\n------------------------------------------------------------\n"; }
void printPlayer(const Player& p) { std::cout << std::left << std::setw(6) << p.number << std::setw(24) << p.name << std::setw(6) << p.age << std::setw(18) << p.nationality << std::setw(14) << p.position << std::setw(8) << p.priority << "\n"; }
void printTable(const std::vector<Player>& players) {
    if (players.empty()) { std::cout << "No roster data available.\n"; return; }
    std::cout << std::left << std::setw(6) << "No." << std::setw(24) << "Name" << std::setw(6) << "Age" << std::setw(18) << "Nationality" << std::setw(14) << "Position" << std::setw(8) << "Priority" << "\n";
    line(); for (const auto& p : players) printPlayer(p);
}
int readInt(const std::string& prompt) { int value; while (true) { std::cout << prompt; if (std::cin >> value) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return value; } std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "Please enter a valid integer.\n"; } }
std::string readText(const std::string& prompt) { std::string value; std::cout << prompt; std::getline(std::cin, value); return trim(value); }
void menu() { line(); std::cout << "RosterFlow | Team Roster Management System\n"; line(); std::cout << "1. List roster\n2. Add player\n3. Find by shirt number\n4. Remove player\n5. Update player priority\n6. Show priority queue\n7. Show next priority player\n8. Undo last addition\n9. Save data\n10. Load data\n0. Exit\n"; }
}

int main() {
    RosterService service; std::string error;
    if (service.loadCsv("data/players.csv", error)) std::cout << "Existing roster data loaded automatically.\n";
    else std::cout << "Starting with an empty roster: " << error << "\n";
    auto autosave = [&]() { std::string saveError; if (!service.saveCsv("data/players.csv", saveError)) std::cout << "Automatic save failed: " << saveError << "\n"; };
    std::cout << "\nWelcome to RosterFlow\nManage your team roster with practical and reliable data structures.\n";
    while (true) {
        menu(); int choice = readInt("Choose an operation: ");
        if (choice == 0) break;
        if (choice == 1) printTable(service.players());
        else if (choice == 2) { Player p; p.number = readInt("Shirt number: "); p.name = readText("Player name: "); p.age = readInt("Age: "); p.nationality = readText("Nationality: "); p.position = readText("Position: "); p.priority = readInt("Priority (1-10): "); if (service.addPlayer(p, error)) { autosave(); std::cout << "Player added successfully and saved automatically.\n"; } else std::cout << error << "\n"; }
        else if (choice == 3) { const auto* p = service.findPlayer(readInt("Shirt number: ")); if (p) printTable({*p}); else std::cout << "Player not found.\n"; }
        else if (choice == 4) { if (service.removePlayer(readInt("Player number to remove: "), error)) { autosave(); std::cout << "Player removed successfully and saved automatically.\n"; } else std::cout << error << "\n"; }
        else if (choice == 5) { int number = readInt("Player number: "); int priority = readInt("New priority (1-10): "); if (service.updatePriority(number, priority, error)) { autosave(); std::cout << "Priority updated successfully and saved automatically.\n"; } else std::cout << error << "\n"; }
        else if (choice == 6) printTable(service.priorityQueue());
        else if (choice == 7) { const auto* p = service.nextPriorityPlayer(); if (p) printTable({*p}); else std::cout << "The priority queue is empty.\n"; }
        else if (choice == 8) { if (service.undoLastAddition(error)) { autosave(); std::cout << "The last addition was undone and saved automatically.\n"; } else std::cout << error << "\n"; }
        else if (choice == 9) { if (service.saveCsv("data/players.csv", error)) std::cout << "Data saved to data/players.csv.\n"; else std::cout << error << "\n"; }
        else if (choice == 10) { if (service.loadCsv("data/players.csv", error)) std::cout << "Data loaded successfully.\n"; else std::cout << error << "\n"; }
        else std::cout << "Unknown option.\n";
    }
    autosave(); std::cout << "Data saved. Goodbye.\n"; return 0;
}

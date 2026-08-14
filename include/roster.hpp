#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace rosterflow {

struct Player {
    int number{};
    std::string name;
    int age{};
    std::string nationality;
    std::string position;
    int priority{};
};

class DoublyLinkedRoster {
public:
    DoublyLinkedRoster() = default;
    DoublyLinkedRoster(const DoublyLinkedRoster&) = delete;
    DoublyLinkedRoster& operator=(const DoublyLinkedRoster&) = delete;
    ~DoublyLinkedRoster();

    bool add(const Player& player);
    bool insertAt(const Player& player, std::size_t index);
    bool removeByNumber(int number);
    Player* findByNumber(int number);
    const Player* findByNumber(int number) const;
    std::vector<Player> toVector() const;
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void clear();

private:
    struct Node {
        Player player;
        Node* previous{};
        Node* next{};
        explicit Node(const Player& value) : player(value) {}
    };

    Node* head_{};
    Node* tail_{};
    std::size_t size_{};
};

class PriorityQueue {
public:
    bool enqueue(const Player& player);
    bool updatePriority(int number, int newPriority);
    bool dequeue(Player& result);
    const Player* peek() const;
    std::vector<Player> ordered() const;
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<Player> items_;
};

class ActionStack {
public:
    void push(const Player& player);
    bool pop(Player& result);
    const Player* peek() const;
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<Player> items_;
};

class RosterService {
public:
    bool addPlayer(Player player, std::string& error);
    bool removePlayer(int number, std::string& error);
    bool updatePriority(int number, int priority, std::string& error);
    const Player* findPlayer(int number) const;
    std::vector<Player> players() const;
    std::vector<Player> priorityQueue() const;
    const Player* nextPriorityPlayer() const;
    const Player* lastAddedPlayer() const;
    bool undoLastAddition(std::string& error);
    bool saveCsv(const std::string& path, std::string& error) const;
    bool loadCsv(const std::string& path, std::string& error);
    std::size_t size() const { return roster_.size(); }

private:
    DoublyLinkedRoster roster_;
    PriorityQueue priorityQueue_;
    ActionStack history_;
};

std::string playerToCsv(const Player& player);
std::string trim(const std::string& value);

} // namespace rosterflow

using rosterflow::ActionStack;
using rosterflow::DoublyLinkedRoster;
using rosterflow::Player;
using rosterflow::PriorityQueue;
using rosterflow::RosterService;
using rosterflow::playerToCsv;
using rosterflow::trim;

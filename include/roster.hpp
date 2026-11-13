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

// ---------------------------------------------------------------------------
// Domain constants and validation (single source of truth for every layer)
// ---------------------------------------------------------------------------
namespace rules {
inline constexpr int kMinAge = 14;
inline constexpr int kMaxAge = 60;
inline constexpr int kMinPriority = 1;
inline constexpr int kMaxPriority = 10;
inline constexpr std::size_t kMaxNameLength = 80;
} // namespace rules

// Field-level validation used by addPlayer and the CSV importer.
bool isValidNumber(int number);
bool isValidName(const std::string& name);
bool isValidAge(int age);
bool isValidPriority(int priority);
// Validates every field and returns the first human-readable problem,
// or an empty string when the player is valid.
std::string validatePlayer(const Player& player);

// ---------------------------------------------------------------------------
// CSV helpers with proper RFC-4180 style quoting/escaping.
// Fixes the historical bug where names containing commas (e.g.
// "Marc-André ter Stegen, Jr.") silently corrupted rows on export/import.
// ---------------------------------------------------------------------------
std::string csvEscape(const std::string& field);
std::string csvUnescape(const std::string& field);
std::vector<std::string> splitCsvLine(const std::string& line);
std::string playerToCsv(const Player& player);
std::string trim(const std::string& value);

// ---------------------------------------------------------------------------
// DoublyLinkedRoster: insertion-ordered roster backed by owned nodes.
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// PriorityQueue: stable ascending order by Player::priority.
// ---------------------------------------------------------------------------
class PriorityQueue {
public:
    bool enqueue(const Player& player);
    bool updatePriority(int number, int newPriority);
    bool dequeue(Player& result);
    bool removeByNumber(int number); // O(n) erase — replaces the old
                                     // "set to INT_MAX then drain" hack.
    const Player* peek() const;
    std::vector<Player> ordered() const;
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    void resort();
    std::vector<Player> items_;
};

// ---------------------------------------------------------------------------
// ActionStack: LIFO history of player additions (undo support).
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Roster statistics (aggregations computed in a single pass).
// ---------------------------------------------------------------------------
struct PositionCount {
    std::string position;
    std::size_t count{};
};

struct SquadStats {
    std::size_t totalPlayers{};
    double averageAge{};
    int youngestNumber{};
    int oldestNumber{};
    std::vector<PositionCount> byPosition; // sorted by count desc, then name
    std::vector<std::string> nationalities; // unique, alphabetical
};

SquadStats computeStats(const std::vector<Player>& players);

// ---------------------------------------------------------------------------
// Report generation (plain text and CSV exports for offline analysis).
// ---------------------------------------------------------------------------
std::string buildTextReport(const std::vector<Player>& players,
                            const std::string& generatedAt);
std::string buildCsvReport(const std::vector<Player>& players);

// ---------------------------------------------------------------------------
// RosterService: application-facing facade coordinating all structures.
// ---------------------------------------------------------------------------
class RosterService {
public:
    bool addPlayer(Player player, std::string& error);
    bool removePlayer(int number, std::string& error);
    bool updatePriority(int number, int priority, std::string& error);
    // Full record edit with re-validation and uniqueness enforcement.
    bool updatePlayer(const Player& updated, std::string& error);
    const Player* findPlayer(int number) const;
    std::vector<Player> players() const;
    std::vector<Player> priorityQueue() const;
    const Player* nextPriorityPlayer() const;
    const Player* lastAddedPlayer() const;
    bool undoLastAddition(std::string& error);
    bool saveCsv(const std::string& path, std::string& error) const;
    bool loadCsv(const std::string& path, std::string& error);
    std::size_t size() const { return roster_.size(); }

    // Advanced filtering / search across name, nationality and position
    // (case-insensitive substring match). Returns results in roster order.
    std::vector<Player> search(const std::string& query) const;
    std::vector<Player> filterByPosition(const std::string& position) const;

    // One-line summary of every shirt-number collision inside the squad.
    std::vector<std::string> integrityIssues() const;

    SquadStats stats() const;

private:
    DoublyLinkedRoster roster_;
    PriorityQueue priorityQueue_;
    ActionStack history_;
};

} // namespace rosterflow

using rosterflow::ActionStack;
using rosterflow::DoublyLinkedRoster;
using rosterflow::Player;
using rosterflow::PriorityQueue;
using rosterflow::RosterService;
using rosterflow::SquadStats;
using rosterflow::csvEscape;
using rosterflow::csvUnescape;
using rosterflow::playerToCsv;
using rosterflow::trim;
using rosterflow::validatePlayer;

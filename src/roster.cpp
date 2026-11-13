#include "roster.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace rosterflow {

// ===========================================================================
// Validation helpers
// ===========================================================================

bool isValidNumber(int number) { return number > 0; }

bool isValidName(const std::string& name) {
    const auto trimmed = trim(name);
    return !trimmed.empty() && trimmed.size() <= rules::kMaxNameLength;
}

bool isValidAge(int age) { return age >= rules::kMinAge && age <= rules::kMaxAge; }

bool isValidPriority(int priority) {
    return priority >= rules::kMinPriority && priority <= rules::kMaxPriority;
}

std::string validatePlayer(const Player& player) {
    if (!isValidNumber(player.number))
        return "Shirt number must be a positive integer.";
    if (!isValidName(player.name))
        return "Player name is required and must be at most " +
               std::to_string(rules::kMaxNameLength) + " characters.";
    if (!isValidAge(player.age))
        return "Age must be between " + std::to_string(rules::kMinAge) + " and " +
               std::to_string(rules::kMaxAge) + ".";
    if (!isValidPriority(player.priority))
        return "Priority must be between " + std::to_string(rules::kMinPriority) +
               " and " + std::to_string(rules::kMaxPriority) + ".";
    return {};
}

// ===========================================================================
// CSV helpers (RFC-4180 style quoting)
// ===========================================================================

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    const auto last = value.find_last_not_of(" \t\r\n");
    return first == std::string::npos ? std::string{} : value.substr(first, last - first + 1);
}

std::string csvEscape(const std::string& field) {
    const bool needsQuotes =
        field.find(',') != std::string::npos ||
        field.find('"') != std::string::npos ||
        field.find('\n') != std::string::npos ||
        field.find('\r') != std::string::npos;
    if (!needsQuotes) return field;
    std::string escaped;
    escaped.reserve(field.size() + 2);
    escaped += '"';
    for (const char c : field) {
        if (c == '"') escaped += "\"\"";
        else escaped += c;
    }
    escaped += '"';
    return escaped;
}

std::string csvUnescape(const std::string& field) {
    const auto value = trim(field);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        std::string inner;
        inner.reserve(value.size() - 2);
        for (std::size_t i = 1; i + 1 < value.size(); ++i) {
            if (value[i] == '"' && i + 2 < value.size() && value[i + 1] == '"') {
                inner += '"';
                ++i; // consume the doubled quote
            } else {
                inner += value[i];
            }
        }
        return inner;
    }
    return value;
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') { current += '"'; ++i; }
                else inQuotes = false;
            } else {
                current += c;
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

std::string playerToCsv(const Player& p) {
    return std::to_string(p.number) + ',' + csvEscape(p.name) + ',' +
           std::to_string(p.age) + ',' + csvEscape(p.nationality) + ',' +
           csvEscape(p.position) + ',' + std::to_string(p.priority);
}

// ===========================================================================
// DoublyLinkedRoster
// ===========================================================================

DoublyLinkedRoster::~DoublyLinkedRoster() { clear(); }

bool DoublyLinkedRoster::add(const Player& player) {
    if (findByNumber(player.number)) return false;
    auto* node = new Node(player);
    if (!tail_) head_ = tail_ = node;
    else { node->previous = tail_; tail_->next = node; tail_ = node; }
    ++size_;
    return true;
}

bool DoublyLinkedRoster::insertAt(const Player& player, std::size_t index) {
    if (index > size_ || findByNumber(player.number)) return false;
    if (index == size_) return add(player);
    auto* current = head_;
    for (std::size_t i = 0; i < index; ++i) current = current->next;
    auto* node = new Node(player);
    node->next = current;
    node->previous = current->previous;
    if (current->previous) current->previous->next = node; else head_ = node;
    current->previous = node;
    ++size_;
    return true;
}

bool DoublyLinkedRoster::removeByNumber(int number) {
    auto* current = head_;
    while (current && current->player.number != number) current = current->next;
    if (!current) return false;
    if (current->previous) current->previous->next = current->next; else head_ = current->next;
    if (current->next) current->next->previous = current->previous; else tail_ = current->previous;
    delete current;
    --size_;
    return true;
}

Player* DoublyLinkedRoster::findByNumber(int number) {
    auto* current = head_;
    while (current && current->player.number != number) current = current->next;
    return current ? &current->player : nullptr;
}

const Player* DoublyLinkedRoster::findByNumber(int number) const {
    auto* current = head_;
    while (current && current->player.number != number) current = current->next;
    return current ? &current->player : nullptr;
}

std::vector<Player> DoublyLinkedRoster::toVector() const {
    std::vector<Player> result;
    result.reserve(size_);
    for (auto* node = head_; node; node = node->next) result.push_back(node->player);
    return result;
}

void DoublyLinkedRoster::clear() {
    while (head_) { auto* next = head_->next; delete head_; head_ = next; }
    tail_ = nullptr;
    size_ = 0;
}

// ===========================================================================
// PriorityQueue
// ===========================================================================

void PriorityQueue::resort() {
    std::stable_sort(items_.begin(), items_.end(),
                     [](const Player& a, const Player& b) { return a.priority < b.priority; });
}

bool PriorityQueue::enqueue(const Player& player) {
    if (std::any_of(items_.begin(), items_.end(),
                    [&](const Player& p) { return p.number == player.number; }))
        return false;
    items_.push_back(player);
    resort();
    return true;
}

bool PriorityQueue::updatePriority(int number, int newPriority) {
    if (!isValidPriority(newPriority)) return false; // enforce the domain rule here too
    for (auto& item : items_) {
        if (item.number == number) {
            item.priority = newPriority;
            resort();
            return true;
        }
    }
    return false;
}

bool PriorityQueue::dequeue(Player& result) {
    if (items_.empty()) return false;
    result = items_.front();
    items_.erase(items_.begin());
    return true;
}

bool PriorityQueue::removeByNumber(int number) {
    const auto it = std::find_if(items_.begin(), items_.end(),
                                 [&](const Player& p) { return p.number == number; });
    if (it == items_.end()) return false;
    items_.erase(it);
    return true;
}

const Player* PriorityQueue::peek() const {
    return items_.empty() ? nullptr : &items_.front();
}

std::vector<Player> PriorityQueue::ordered() const { return items_; }

// ===========================================================================
// ActionStack
// ===========================================================================

void ActionStack::push(const Player& player) { items_.push_back(player); }

bool ActionStack::pop(Player& result) {
    if (items_.empty()) return false;
    result = items_.back();
    items_.pop_back();
    return true;
}

const Player* ActionStack::peek() const {
    return items_.empty() ? nullptr : &items_.back();
}

// ===========================================================================
// Statistics
// ===========================================================================

SquadStats computeStats(const std::vector<Player>& players) {
    SquadStats stats;
    stats.totalPlayers = players.size();
    if (players.empty()) return stats;

    long long ageSum = 0;
    int youngest = players.front().age;
    int oldest = players.front().age;
    stats.youngestNumber = players.front().number;
    stats.oldestNumber = players.front().number;

    std::map<std::string, std::size_t> positionCounts;
    std::set<std::string> nationalities;

    for (const auto& p : players) {
        ageSum += p.age;
        if (p.age < youngest) { youngest = p.age; stats.youngestNumber = p.number; }
        if (p.age > oldest)   { oldest = p.age;   stats.oldestNumber = p.number; }
        positionCounts[p.position.empty() ? "Unspecified" : p.position]++;
        if (!p.nationality.empty()) nationalities.insert(p.nationality);
    }

    stats.averageAge = static_cast<double>(ageSum) / static_cast<double>(players.size());

    for (const auto& entry : positionCounts)
        stats.byPosition.push_back({entry.first, entry.second});
    std::sort(stats.byPosition.begin(), stats.byPosition.end(),
              [](const PositionCount& a, const PositionCount& b) {
                  if (a.count != b.count) return a.count > b.count;
                  return a.position < b.position;
              });

    stats.nationalities.assign(nationalities.begin(), nationalities.end());
    return stats;
}

// ===========================================================================
// Reports
// ===========================================================================

namespace {
std::string repeat(char c, std::size_t n) { return std::string(n, c); }
} // namespace

std::string buildTextReport(const std::vector<Player>& players,
                            const std::string& generatedAt) {
    std::ostringstream out;
    out << "============================================================\n";
    out << " ROSTERFLOW SQUAD REPORT\n";
    out << " Generated: " << generatedAt << "\n";
    out << "============================================================\n\n";

    if (players.empty()) {
        out << "The roster is empty. No data to report.\n";
        return out.str();
    }

    const auto stats = computeStats(players);
    out << "SUMMARY\n-------\n";
    out << "Total players : " << stats.totalPlayers << "\n";
    out << "Average age   : " << (static_cast<int>(stats.averageAge * 100) / 100.0) << "\n";
    for (const auto& pc : stats.byPosition)
        out << "  - " << pc.position << ": " << pc.count << "\n";
    out << "Nationalities : ";
    for (std::size_t i = 0; i < stats.nationalities.size(); ++i) {
        if (i) out << ", ";
        out << stats.nationalities[i];
    }
    out << "\n\n";

    out << "PRIORITY SELECTION ORDER (lowest priority value first)\n";
    out << repeat('-', 60) << "\n";
    auto ordered = players;
    std::stable_sort(ordered.begin(), ordered.end(),
                     [](const Player& a, const Player& b) { return a.priority < b.priority; });
    for (const auto& p : ordered) {
        out << "#" << p.number << " " << p.name << " (" << p.position
            << ", priority " << p.priority << ")\n";
    }
    return out.str();
}

std::string buildCsvReport(const std::vector<Player>& players) {
    std::ostringstream out;
    out << "number,name,age,nationality,position,priority\n";
    for (const auto& p : players) out << playerToCsv(p) << "\n";
    return out.str();
}

// ===========================================================================
// RosterService
// ===========================================================================

bool RosterService::addPlayer(Player player, std::string& error) {
    player.name = trim(player.name);
    player.nationality = trim(player.nationality);
    player.position = trim(player.position);

    const auto problem = validatePlayer(player);
    if (!problem.empty()) { error = problem; return false; }

    // All-or-nothing: enqueue into the queue first (duplicate check), then
    // commit to the roster. The previous code could leave the two structures
    // out of sync when only one insertion succeeded.
    if (!priorityQueue_.enqueue(player)) {
        error = "That player number is already in use.";
        return false;
    }
    if (!roster_.add(player)) {
        priorityQueue_.removeByNumber(player.number); // roll back
        error = "That player number is already in use.";
        return false;
    }
    history_.push(player);
    error.clear();
    return true;
}

bool RosterService::removePlayer(int number, std::string& error) {
    if (!roster_.removeByNumber(number)) {
        error = "No player was found with that number.";
        return false;
    }
    priorityQueue_.removeByNumber(number); // direct O(n) erase (old code
                                           // resorted the whole queue twice)
    error.clear();
    return true;
}

bool RosterService::updatePriority(int number, int priority, std::string& error) {
    if (!isValidPriority(priority)) {
        error = "Priority must be between " + std::to_string(rules::kMinPriority) +
                " and " + std::to_string(rules::kMaxPriority) + ".";
        return false;
    }
    if (!priorityQueue_.updatePriority(number, priority)) {
        error = "No player was found with that number.";
        return false;
    }
    if (auto* player = roster_.findByNumber(number)) player->priority = priority;
    error.clear();
    return true;
}

bool RosterService::updatePlayer(const Player& updated, std::string& error) {
    Player normalized = updated;
    normalized.name = trim(normalized.name);
    normalized.nationality = trim(normalized.nationality);
    normalized.position = trim(normalized.position);

    const auto problem = validatePlayer(normalized);
    if (!problem.empty()) { error = problem; return false; }

    const auto* existing = roster_.findByNumber(normalized.number);
    if (!existing) { error = "No player was found with that number."; return false; }

    // Remove the stale record from both structures, then re-add the edited
    // copy so ordering semantics stay consistent (queue re-sorts by priority).
    Player old = *existing;
    removePlayer(old.number, error);
    if (!addPlayer(normalized, error)) {
        addPlayer(old, error); // best-effort rollback of the failed edit
        if (!error.empty()) error = "Edit rejected: " + error;
        return false;
    }
    error.clear();
    return true;
}

const Player* RosterService::findPlayer(int number) const {
    return roster_.findByNumber(number);
}

std::vector<Player> RosterService::players() const { return roster_.toVector(); }

std::vector<Player> RosterService::priorityQueue() const { return priorityQueue_.ordered(); }

const Player* RosterService::nextPriorityPlayer() const { return priorityQueue_.peek(); }

const Player* RosterService::lastAddedPlayer() const { return history_.peek(); }

bool RosterService::undoLastAddition(std::string& error) {
    Player player;
    if (!history_.pop(player)) {
        error = "There is no addition to undo.";
        return false;
    }
    roster_.removeByNumber(player.number);
    priorityQueue_.removeByNumber(player.number);
    error.clear();
    return true;
}

bool RosterService::saveCsv(const std::string& path, std::string& error) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file) { error = "Could not open the save file: " + path; return false; }
    file << "number,name,age,nationality,position,priority\n";
    for (const auto& p : players()) file << playerToCsv(p) << "\n";
    file.flush();
    if (file.fail()) { error = "Disk write failed while saving " + path; return false; }
    error.clear();
    return true;
}

bool RosterService::loadCsv(const std::string& path, std::string& error) {
    std::ifstream file(path);
    if (!file) { error = "Could not open the data file: " + path; return false; }

    std::string line;
    std::getline(file, line); // header (ignored but tolerated if absent)

    DoublyLinkedRoster roster;
    PriorityQueue queue;
    std::size_t row = 1;
    while (std::getline(file, line)) {
        ++row;
        if (trim(line).empty()) continue;
        auto fields = splitCsvLine(line);
        if (fields.size() != 6) {
            error = "Invalid CSV file: row " + std::to_string(row) +
                    " has " + std::to_string(fields.size()) + " fields, expected 6.";
            return false;
        }
        for (auto& f : fields) f = csvUnescape(f);

        Player p;
        try {
            p = Player{std::stoi(fields[0]), fields[1], std::stoi(fields[2]),
                       fields[3], fields[4], std::stoi(fields[5])};
        } catch (const std::exception&) {
            error = "The file contains an invalid number on row " + std::to_string(row) + ".";
            return false;
        }
        const auto problem = validatePlayer(p);
        if (!problem.empty()) {
            error = "Row " + std::to_string(row) + ": " + problem;
            return false;
        }
        if (!roster.add(p) || !queue.enqueue(p)) {
            error = "Duplicate data found in the file on row " + std::to_string(row) + ".";
            return false;
        }
    }

    // Atomic swap: the live state is only replaced after the whole file has
    // parsed successfully, so a corrupt file can never wipe the working roster.
    roster_.clear();
    priorityQueue_ = PriorityQueue{};
    history_ = ActionStack{};
    for (const auto& p : roster.toVector()) {
        roster_.add(p);
        priorityQueue_.enqueue(p);
    }
    error.clear();
    return true;
}

// ===========================================================================
// Search / filtering / integrity
// ===========================================================================

namespace {
std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
} // namespace

std::vector<Player> RosterService::search(const std::string& query) const {
    const auto needle = toLower(trim(query));
    std::vector<Player> results;
    if (needle.empty()) return results;
    for (const auto& p : roster_.toVector()) {
        const bool match =
            toLower(p.name).find(needle) != std::string::npos ||
            toLower(p.nationality).find(needle) != std::string::npos ||
            toLower(p.position).find(needle) != std::string::npos;
        if (match) results.push_back(p);
    }
    return results;
}

std::vector<Player> RosterService::filterByPosition(const std::string& position) const {
    const auto needle = toLower(trim(position));
    std::vector<Player> results;
    for (const auto& p : roster_.toVector()) {
        if (toLower(p.position) == needle) results.push_back(p);
    }
    return results;
}

std::vector<std::string> RosterService::integrityIssues() const {
    std::vector<std::string> issues;
    const auto players = roster_.toVector();
    std::map<int, std::size_t> numberCount;
    for (const auto& p : players) numberCount[p.number]++;
    for (const auto& entry : numberCount) {
        if (entry.second > 1) {
            issues.push_back("Shirt number " + std::to_string(entry.first) +
                             " is claimed by " + std::to_string(entry.second) + " records.");
        }
    }
    if (priorityQueue_.size() != players.size()) {
        issues.push_back("Internal inconsistency: the priority queue holds " +
                         std::to_string(priorityQueue_.size()) + " entries but the roster holds " +
                         std::to_string(players.size()) + ".");
    }
    return issues;
}

SquadStats RosterService::stats() const { return computeStats(roster_.toVector()); }

} // namespace rosterflow

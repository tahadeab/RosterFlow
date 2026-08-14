#include "roster.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace rosterflow {

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
    node->next = current; node->previous = current->previous;
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
    delete current; --size_; return true;
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
    std::vector<Player> result; result.reserve(size_);
    for (auto* node = head_; node; node = node->next) result.push_back(node->player);
    return result;
}

void DoublyLinkedRoster::clear() {
    while (head_) { auto* next = head_->next; delete head_; head_ = next; }
    tail_ = nullptr; size_ = 0;
}

bool PriorityQueue::enqueue(const Player& player) {
    if (std::any_of(items_.begin(), items_.end(), [&](const Player& p) { return p.number == player.number; })) return false;
    items_.push_back(player);
    std::stable_sort(items_.begin(), items_.end(), [](const Player& a, const Player& b) { return a.priority < b.priority; });
    return true;
}

bool PriorityQueue::updatePriority(int number, int newPriority) {
    for (auto& item : items_) if (item.number == number) { item.priority = newPriority; std::stable_sort(items_.begin(), items_.end(), [](const Player& a, const Player& b) { return a.priority < b.priority; }); return true; }
    return false;
}

bool PriorityQueue::dequeue(Player& result) { if (items_.empty()) return false; result = items_.front(); items_.erase(items_.begin()); return true; }
const Player* PriorityQueue::peek() const { return items_.empty() ? nullptr : &items_.front(); }
std::vector<Player> PriorityQueue::ordered() const { return items_; }
void ActionStack::push(const Player& player) { items_.push_back(player); }
bool ActionStack::pop(Player& result) { if (items_.empty()) return false; result = items_.back(); items_.pop_back(); return true; }
const Player* ActionStack::peek() const { return items_.empty() ? nullptr : &items_.back(); }

bool RosterService::addPlayer(Player player, std::string& error) {
    if (player.number <= 0 || player.name.empty() || player.age < 14 || player.age > 60 || player.priority < 1) { error = "البيانات غير صالحة: تأكد من الرقم والاسم والعمر والأولوية."; return false; }
    if (!roster_.add(player) || !priorityQueue_.enqueue(player)) { error = "رقم اللاعب مستخدم مسبقاً."; return false; }
    history_.push(player); return true;
}

bool RosterService::removePlayer(int number, std::string& error) {
    if (!roster_.removeByNumber(number)) { error = "لم يتم العثور على لاعب بهذا الرقم."; return false; }
    priorityQueue_.updatePriority(number, 1000000); Player ignored; while (priorityQueue_.dequeue(ignored) && ignored.number != number) {}
    return true;
}

bool RosterService::updatePriority(int number, int priority, std::string& error) {
    if (priority < 1 || priority > 10 || !priorityQueue_.updatePriority(number, priority)) { error = "تعذر تحديث الأولوية. يجب أن تكون بين 1 و10."; return false; }
    if (auto* player = roster_.findByNumber(number)) player->priority = priority;
    return true;
}

const Player* RosterService::findPlayer(int number) const { return roster_.findByNumber(number); }
std::vector<Player> RosterService::players() const { return roster_.toVector(); }
std::vector<Player> RosterService::priorityQueue() const { return priorityQueue_.ordered(); }
const Player* RosterService::nextPriorityPlayer() const { return priorityQueue_.peek(); }
const Player* RosterService::lastAddedPlayer() const { return history_.peek(); }

bool RosterService::undoLastAddition(std::string& error) {
    Player player; if (!history_.pop(player) || !roster_.removeByNumber(player.number)) { error = "لا توجد عملية إضافة يمكن التراجع عنها."; return false; }
    Player current; std::vector<Player> remaining; while (priorityQueue_.dequeue(current)) if (current.number != player.number) remaining.push_back(current);
    for (const auto& item : remaining) priorityQueue_.enqueue(item);
    return true;
}

std::string playerToCsv(const Player& p) { return std::to_string(p.number) + "," + p.name + "," + std::to_string(p.age) + "," + p.nationality + "," + p.position + "," + std::to_string(p.priority); }

bool RosterService::saveCsv(const std::string& path, std::string& error) const {
    std::ofstream file(path); if (!file) { error = "تعذر فتح ملف الحفظ."; return false; }
    file << "number,name,age,nationality,position,priority\n"; for (const auto& p : players()) file << playerToCsv(p) << "\n"; return true;
}

bool RosterService::loadCsv(const std::string& path, std::string& error) {
    std::ifstream file(path); if (!file) { error = "تعذر فتح ملف البيانات."; return false; }
    std::string line; std::getline(file, line); DoublyLinkedRoster roster; PriorityQueue queue;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;
        while (std::getline(ss, field, ',')) fields.push_back(trim(field));
        if (fields.size() != 6) { error = "ملف CSV غير صحيح."; return false; }
        try { Player p{std::stoi(fields[0]), fields[1], std::stoi(fields[2]), fields[3], fields[4], std::stoi(fields[5])}; if (!roster.add(p) || !queue.enqueue(p)) { error = "بيانات مكررة في الملف."; return false; } } catch (...) { error = "يوجد رقم غير صالح في الملف."; return false; }
    }
    roster_.clear();
    priorityQueue_ = PriorityQueue{};
    for (const auto& p : roster.toVector()) { roster_.add(p); priorityQueue_.enqueue(p); }
    return true;
}

std::string trim(const std::string& value) { const auto first = value.find_first_not_of(" \t\r\n"); const auto last = value.find_last_not_of(" \t\r\n"); return first == std::string::npos ? "" : value.substr(first, last - first + 1); }

} // namespace rosterflow

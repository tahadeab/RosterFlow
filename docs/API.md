# RosterFlow API Documentation

## Overview

This document describes the public C++ API exposed by RosterFlow. The API is organized around a `Player` model, three educational data structures, and a `RosterService` facade that coordinates roster management and CSV persistence.

## Player

```cpp
struct Player {
    int number;
    std::string name;
    int age;
    std::string nationality;
    std::string position;
    int priority;
};
```

`number` is the unique shirt number. `name`, `nationality`, and `position` are descriptive fields. `age` must be between 14 and 60 when a player is added through `RosterService`. `priority` is a positive integer; the application UI uses the range 1–10, where a lower value represents a higher selection priority.

## DoublyLinkedRoster

`DoublyLinkedRoster` stores players in insertion order using nodes with previous and next links. It owns its nodes and releases them in its destructor.

| Method | Description | Return value |
|---|---|---|
| `bool add(const Player&)` | Appends a player unless the shirt number already exists. | `true` on success |
| `bool insertAt(const Player&, size_t)` | Inserts at a zero-based index. An index equal to `size()` appends. | `true` on success |
| `bool removeByNumber(int)` | Removes the player with the requested shirt number. | `true` if removed |
| `Player* findByNumber(int)` | Searches for a mutable player record. | Pointer or `nullptr` |
| `const Player* findByNumber(int) const` | Searches in a read-only roster. | Pointer or `nullptr` |
| `std::vector<Player> toVector() const` | Creates a snapshot in roster order. | Player vector |
| `size_t size() const` | Returns the number of stored players. | Element count |
| `bool empty() const` | Checks whether the roster has no players. | Boolean |
| `void clear()` | Deletes all nodes and resets the roster. | None |

## PriorityQueue

`PriorityQueue` maintains a stable ascending order by `Player::priority`. Players with smaller priority values are returned first. Equal priorities retain insertion order.

| Method | Description | Return value |
|---|---|---|
| `bool enqueue(const Player&)` | Adds a unique player and reorders the queue. | `true` on success |
| `bool updatePriority(int, int)` | Updates a player priority and restores ordering. | `true` if found |
| `bool dequeue(Player&)` | Removes and copies the highest-priority player. | `true` if non-empty |
| `const Player* peek() const` | Reads the next player without removing it. | Pointer or `nullptr` |
| `std::vector<Player> ordered() const` | Returns an ordered snapshot. | Player vector |
| `size_t size() const` | Returns the queue size. | Element count |
| `bool empty() const` | Checks whether the queue is empty. | Boolean |

## ActionStack

`ActionStack` provides last-in, first-out storage for player additions. RosterFlow uses it to undo the most recent addition.

| Method | Description | Return value |
|---|---|---|
| `void push(const Player&)` | Adds a player to the top of the stack. | None |
| `bool pop(Player&)` | Removes and copies the latest player. | `true` if non-empty |
| `const Player* peek() const` | Reads the latest player without removing it. | Pointer or `nullptr` |
| `size_t size() const` | Returns the stack size. | Element count |
| `bool empty() const` | Checks whether the stack is empty. | Boolean |

## RosterService

`RosterService` is the recommended application-facing API. It keeps the linked roster, priority queue, and action history synchronized.

| Method | Description |
|---|---|
| `bool addPlayer(Player, string&)` | Validates and adds a player to the roster and priority queue. The error string is populated on failure. |
| `bool removePlayer(int, string&)` | Removes a player from both managed structures. |
| `bool updatePriority(int, int, string&)` | Updates a player's priority in the queue and roster. |
| `const Player* findPlayer(int) const` | Finds a player by shirt number. |
| `vector<Player> players() const` | Returns players in roster order. |
| `vector<Player> priorityQueue() const` | Returns players sorted by priority. |
| `const Player* nextPriorityPlayer() const` | Returns the next player without dequeuing. |
| `const Player* lastAddedPlayer() const` | Returns the latest addition available for undo. |
| `bool undoLastAddition(string&)` | Removes the latest addition and synchronizes the priority queue. |
| `bool saveCsv(const string&, string&) const` | Writes a header and all players to a CSV file. |
| `bool loadCsv(const string&, string&)` | Reads a CSV file, validates its rows, and replaces the current state. |
| `size_t size() const` | Returns the current roster size. |

### Validation Rules

`addPlayer` rejects non-positive shirt numbers, empty names, ages outside 14–60, and priorities below 1. It also rejects duplicate shirt numbers. `updatePriority` accepts values from 1 to 10 in the service layer. `loadCsv` rejects malformed rows, invalid numbers, and duplicate players.

### Persistence Workflow

The command-line application calls `loadCsv("data/players.csv", error)` before displaying the menu. If the file exists and is valid, the roster is restored automatically. After every successful add, remove, priority update, or undo operation, the application calls `saveCsv` automatically. It also performs a final save when the application exits. Manual Save and Load menu options remain available.

The CSV schema is:

```text
number,name,age,nationality,position,priority
```

Names and text fields should not contain commas unless CSV escaping is added in a future version.

## Utility Functions

`playerToCsv(const Player&)` serializes one player into the project CSV format. `trim(const std::string&)` removes leading and trailing whitespace from a field during import.

## Error Handling

The API uses boolean success flags rather than exceptions for expected validation and file errors. When an operation returns `false`, its `std::string& error` argument contains a user-readable explanation. Pointer-returning lookup methods return `nullptr` when an item does not exist.

## Example

```cpp
RosterService service;
std::string error;

service.loadCsv("data/players.csv", error);
service.addPlayer({10, "Example Player", 22, "Testland", "Midfielder", 2}, error);
service.updatePriority(10, 1, error);

if (const Player* next = service.nextPriorityPlayer()) {
    std::cout << next->name << '\n';
}

service.saveCsv("data/players.csv", error);
```

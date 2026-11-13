#include "roster.hpp"
#include "config.hpp"
#include "logger.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace rosterflow;

namespace {

// ---------------------------------------------------------------------------
// Terminal UI helpers (columns adapt to the widest name in the result set)
// ---------------------------------------------------------------------------

void rule(char c = '-', std::size_t width = 78) { std::cout << std::string(width, c) << "\n"; }

std::size_t displayWidth(const Player& p) { return p.name.size(); }

void printTable(const std::vector<Player>& players) {
    if (players.empty()) {
        std::cout << "[empty] No players match. Use option 2 to add a player or option 13\n"
                  << "        to import records from a CSV file.\n";
        return;
    }
    std::size_t nameWidth = 4;
    for (const auto& p : players) nameWidth = std::max(nameWidth, displayWidth(p));
    nameWidth = std::min<std::size_t>(nameWidth + 2, 36);

    std::cout << std::left
              << std::setw(6) << "No."
              << std::setw(static_cast<int>(nameWidth)) << "Name"
              << std::setw(6) << "Age"
              << std::setw(18) << "Nationality"
              << std::setw(14) << "Position"
              << std::setw(9) << "Priority" << "\n";
    rule('-', 6 + nameWidth + 6 + 18 + 14 + 9);
    for (const auto& p : players) {
        std::string name = p.name.size() > 34 ? p.name.substr(0, 33) + "~" : p.name;
        std::cout << std::left
                  << std::setw(6) << p.number
                  << std::setw(static_cast<int>(nameWidth)) << name
                  << std::setw(6) << p.age
                  << std::setw(18) << p.nationality
                  << std::setw(14) << p.position
                  << std::setw(9) << p.priority << "\n";
    }
}

void success(const std::string& message) { std::cout << "[ok] " << message << "\n"; }
void failure(const std::string& message) { std::cout << "[error] " << message << "\n"; }

int readInt(const std::string& prompt) {
    int value{};
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Please enter a valid integer.\n";
    }
}

std::string readText(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return trim(value);
}

std::string nowUtc() {
    const auto seconds = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif
    std::ostringstream stamp;
    stamp << std::put_time(&utc, "%Y-%m-%d %H:%M:%S UTC");
    return stamp.str();
}

void menu() {
    rule('=');
    std::cout
        << "RosterFlow | Team Roster Management System\n"
        << "------------------------------------------\n"
        << " Roster                Views                 Data\n"
        << "  1. List roster         6. Priority queue     10. Save now\n"
        << "  2. Add player          7. Next pick          11. Reload file\n"
        << "  3. Edit player         8. Squad stats        12. Export report\n"
        << "  4. Remove player       9. Search / filter    13. Import CSV\n"
        << "  5. Set priority        14. Undo last add     15. Integrity check\n"
        << "  0. Exit (auto-saves)\n";
    rule('=');
}

// ---------------------------------------------------------------------------
// Feature flows
// ---------------------------------------------------------------------------

void flowAdd(RosterService& service, const AppConfig& config) {
    Player p;
    p.number = readInt("Shirt number: ");
    p.name = readText("Player name: ");
    p.age = readInt("Age: ");
    p.nationality = readText("Nationality: ");
    p.position = readText("Position: ");
    p.priority = readInt("Priority (1-10, 1 = first pick): ");
    std::string error;
    if (service.addPlayer(p, error)) {
        Logger::instance().info("add player #" + std::to_string(p.number) + " '" + p.name + "'");
        success("Player added.");
        if (config.autosave) service.saveCsv(config.dataFile.string(), error);
    } else {
        Logger::instance().warn("add rejected: " + error);
        failure(error);
    }
}

void flowEdit(RosterService& service, const AppConfig& config) {
    const int number = readInt("Shirt number of the player to edit: ");
    const auto* found = service.findPlayer(number);
    if (!found) { failure("No player was found with that number."); return; }
    Player updated = *found;
    std::cout << "Editing #" << number << " " << updated.name
              << " — press Enter to keep the current value.\n";

    auto keepOrEdit = [&](const std::string& label, std::string& field) {
        const auto input = readText(label + " [" + field + "]: ");
        if (!input.empty()) field = input;
    };
    auto keepOrEditInt = [&](const std::string& label, int& field) {
        const auto input = readText(label + " [" + std::to_string(field) + "] (Enter keeps): ");
        if (!input.empty()) {
            try { field = std::stoi(input); }
            catch (const std::exception&) { std::cout << "Not a number; keeping " << field << ".\n"; }
        }
    };

    keepOrEdit("Name", updated.name);
    keepOrEditInt("Age", updated.age);
    keepOrEdit("Nationality", updated.nationality);
    keepOrEdit("Position", updated.position);
    keepOrEditInt("Priority (1-10)", updated.priority);

    std::string error;
    if (service.updatePlayer(updated, error)) {
        Logger::instance().info("edit player #" + std::to_string(number));
        success("Player record updated.");
        if (config.autosave) service.saveCsv(config.dataFile.string(), error);
    } else {
        Logger::instance().warn("edit rejected: " + error);
        failure(error);
    }
}

void flowRemove(RosterService& service, const AppConfig& config) {
    const int number = readInt("Player number to remove: ");
    std::string error;
    if (service.removePlayer(number, error)) {
        Logger::instance().info("remove player #" + std::to_string(number));
        success("Player removed.");
        if (config.autosave) service.saveCsv(config.dataFile.string(), error);
    } else {
        failure(error);
    }
}

void flowPriority(RosterService& service, const AppConfig& config) {
    const int number = readInt("Player number: ");
    const int priority = readInt("New priority (1-10): ");
    std::string error;
    if (service.updatePriority(number, priority, error)) {
        Logger::instance().info("priority #" + std::to_string(number) + " -> " +
                                std::to_string(priority));
        success("Priority updated.");
        if (config.autosave) service.saveCsv(config.dataFile.string(), error);
    } else {
        failure(error);
    }
}

void flowUndo(RosterService& service, const AppConfig& config) {
    std::string error;
    if (service.undoLastAddition(error)) {
        Logger::instance().info("undo last addition");
        success("The last addition was undone.");
        if (config.autosave) service.saveCsv(config.dataFile.string(), error);
    } else {
        failure(error);
    }
}

void flowSearch(const RosterService& service) {
    const auto query = readText("Search text (name / nationality / position, blank = cancel): ");
    if (query.empty()) { std::cout << "Search cancelled.\n"; return; }
    std::cout << "Results for \"" << query << "\":\n";
    printTable(service.search(query));
}

void flowStats(const RosterService& service) {
    const auto stats = service.stats();
    if (stats.totalPlayers == 0) { printTable({}); return; }
    std::cout << "Squad statistics\n" << std::string(24, '-') << "\n"
              << "Total players : " << stats.totalPlayers << "\n"
              << std::fixed << std::setprecision(1)
              << "Average age   : " << stats.averageAge << "\n";
    if (const auto* youngest = service.findPlayer(stats.youngestNumber))
        std::cout << "Youngest      : #" << youngest->number << " " << youngest->name
                  << " (" << youngest->age << ")\n";
    if (const auto* oldest = service.findPlayer(stats.oldestNumber))
        std::cout << "Oldest        : #" << oldest->number << " " << oldest->name
                  << " (" << oldest->age << ")\n";
    std::cout << "By position   :\n";
    for (const auto& pc : stats.byPosition)
        std::cout << "   " << std::left << std::setw(16) << pc.position << pc.count << "\n";
    std::cout << "Nationalities : ";
    for (std::size_t i = 0; i < stats.nationalities.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << stats.nationalities[i];
    }
    std::cout << "\n";
}

void flowExport(const RosterService& service) {
    const auto base = readText("Export filename [report.csv]: ");
    const std::string path = base.empty() ? "report.csv" : base;
    std::ofstream out(path);
    if (!out) { failure("Could not create " + path); return; }
    out << buildTextReport(service.players(), nowUtc());
    success("Readable squad report exported to " + path);
}

void flowImport(RosterService& service, const AppConfig& config) {
    const auto path = readText("CSV file to merge into the roster: ");
    if (path.empty()) { std::cout << "Import cancelled.\n"; return; }
    std::ifstream probe(path);
    if (!probe) { failure("Could not open " + path); return; }

    RosterService imported;
    std::string error;
    if (!imported.loadCsv(path, error)) { failure("Import failed: " + error); return; }

    int added = 0;
    int skipped = 0;
    for (const auto& p : imported.players()) {
        if (service.addPlayer(p, error)) ++added;
        else ++skipped; // duplicates or invalid rows never corrupt live data
    }
    Logger::instance().info("merge import " + path + ": +" + std::to_string(added) +
                            " skipped " + std::to_string(skipped));
    success("Import complete: " + std::to_string(added) + " added, " +
            std::to_string(skipped) + " skipped (already present or invalid).");
    if (added > 0 && config.autosave) service.saveCsv(config.dataFile.string(), error);
}

void flowIntegrity(const RosterService& service) {
    const auto issues = service.integrityIssues();
    if (issues.empty()) {
        success("Integrity check passed: numbers are unique and all structures are in sync.");
    } else {
        failure("Found " + std::to_string(issues.size()) + " issue(s):");
        for (const auto& issue : issues) std::cout << "   - " << issue << "\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    AppConfig config = AppConfig::fromEnvironment();

    // Command-line overrides take precedence over environment variables.
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--data" || arg == "-d") && i + 1 < argc) config.dataFile = argv[++i];
        else if ((arg == "--log" || arg == "-l") && i + 1 < argc) config.auditLogFile = argv[++i];
        else if (arg == "--no-autosave") config.autosave = false;
        else if (arg == "--no-log") config.loggingEnabled = false;
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: rosterflow [--data FILE] [--log FILE] [--no-autosave] [--no-log]\n"
                      << "Environment: ROSTERFLOW_DATA_FILE, ROSTERFLOW_LOG_FILE,\n"
                      << "             ROSTERFLOW_AUTOSAVE, ROSTERFLOW_LOG_ENABLED\n";
            return 0;
        } else {
            std::cout << "Unknown option: " << arg << " (try --help)\n";
        }
    }

    Logger::instance().setEnabled(config.loggingEnabled);
    Logger::instance().setLogFile(config.auditLogFile);

    RosterService service;
    std::string error;
    if (service.loadCsv(config.dataFile.string(), error)) {
        std::cout << "Roster loaded from " << config.dataFile.string() << " ("
                  << service.size() << " players).\n";
        Logger::instance().info("startup load " + config.dataFile.string() + " size=" +
                                std::to_string(service.size()));
    } else {
        std::cout << "Starting with an empty roster (" << error << ").\n";
        Logger::instance().warn("startup load failed: " + error);
    }

    std::cout << "\nWelcome to RosterFlow — manage your team roster with practical,\n"
              << "reliable data structures. Autosave is "
              << (config.autosave ? "ON" : "OFF") << ".\n";

    bool running = true;
    while (running) {
        menu();
        const int choice = readInt("Choose an operation: ");
        switch (choice) {
            case 0: running = false; break;
            case 1: printTable(service.players()); break;
            case 2: flowAdd(service, config); break;
            case 3: flowEdit(service, config); break;
            case 4: flowRemove(service, config); break;
            case 5: flowPriority(service, config); break;
            case 6: printTable(service.priorityQueue()); break;
            case 7: {
                const auto* p = service.nextPriorityPlayer();
                if (p) printTable({*p});
                else printTable({});
                break;
            }
            case 8: flowStats(service); break;
            case 9: flowSearch(service); break;
            case 10:
                if (service.saveCsv(config.dataFile.string(), error)) {
                    Logger::instance().info("manual save");
                    success("Data saved to " + config.dataFile.string() + ".");
                } else failure(error);
                break;
            case 11:
                if (service.loadCsv(config.dataFile.string(), error)) {
                    Logger::instance().info("manual reload");
                    success("Data reloaded from disk.");
                } else failure(error);
                break;
            case 12: flowExport(service); break;
            case 13: flowImport(service, config); break;
            case 14: flowUndo(service, config); break;
            case 15: flowIntegrity(service); break;
            default: std::cout << "Unknown option. Choose 0-15.\n";
        }
    }

    std::string finalError;
    if (service.saveCsv(config.dataFile.string(), finalError)) {
        Logger::instance().info("shutdown save size=" + std::to_string(service.size()));
        std::cout << "Data saved. Goodbye.\n";
    } else {
        Logger::instance().error("shutdown save failed: " + finalError);
        std::cout << "Final save failed: " << finalError << "\n";
    }
    return 0;
}

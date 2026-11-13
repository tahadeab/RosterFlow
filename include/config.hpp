#pragma once

#include <filesystem>
#include <string>

namespace rosterflow {

// Centralised, environment-driven configuration.
// Every value can be overridden through a ROSTERFLOW_* environment variable,
// which keeps deployment environments (dev / staging / production) cleanly
// separated from code and avoids hard-coded paths.
struct AppConfig {
    std::filesystem::path dataFile{"data/players.csv"};
    std::filesystem::path auditLogFile{"data/rosterflow.log"};
    bool autosave = true;
    bool loggingEnabled = true;

    // Loads the configuration from environment variables with safe defaults:
    //   ROSTERFLOW_DATA_FILE   - path of the CSV roster file
    //   ROSTERFLOW_LOG_FILE    - path of the append-only audit log
    //   ROSTERFLOW_AUTOSAVE    - "0"/"false"/"off" disables automatic saving
    //   ROSTERFLOW_LOG_ENABLED - "0"/"false"/"off" disables audit logging
    static AppConfig fromEnvironment();

    // Human-readable summary used at startup and by the Settings menu.
    std::string describe() const;
};

} // namespace rosterflow

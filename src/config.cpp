#include "config.hpp"

#include <cstdlib>

namespace rosterflow {

namespace {

const char* envValue(const char* name) {
    const char* value = std::getenv(name);
    return (value && *value) ? value : nullptr;
}

bool isTruthy(const char* value) {
    if (!value) return true; // absent means "keep default on"
    std::string lower;
    for (const char* p = value; *p; ++p)
        lower += static_cast<char>(((*p >= 'A' && *p <= 'Z') ? (*p + 32) : *p));
    return !(lower == "0" || lower == "false" || lower == "off" || lower == "no");
}

std::filesystem::path pathOr(const char* name, const std::filesystem::path& fallback) {
    const char* value = envValue(name);
    return value ? std::filesystem::path(value) : fallback;
}

} // namespace

AppConfig AppConfig::fromEnvironment() {
    AppConfig config;
    config.dataFile = pathOr("ROSTERFLOW_DATA_FILE", config.dataFile);
    config.auditLogFile = pathOr("ROSTERFLOW_LOG_FILE", config.auditLogFile);
    config.autosave = isTruthy(envValue("ROSTERFLOW_AUTOSAVE"));
    config.loggingEnabled = isTruthy(envValue("ROSTERFLOW_LOG_ENABLED"));
    return config;
}

std::string AppConfig::describe() const {
    std::string text = "data file: " + dataFile.string();
    text += "\nlog file: " + auditLogFile.string();
    text += "\nautosave: " + std::string(autosave ? "on" : "off");
    text += "\naudit logging: " + std::string(loggingEnabled ? "on" : "off");
    return text;
}

} // namespace rosterflow

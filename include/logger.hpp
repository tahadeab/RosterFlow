#pragma once

#include <filesystem>
#include <mutex>
#include <string>

namespace rosterflow {

enum class LogLevel { Info, Warn, Error };

// Thread-safe, best-effort audit logger.
// Log failures must never crash or block the application; they are silently
// swallowed after an optional one-time warning on stderr.
class Logger {
public:
    static Logger& instance();

    void setEnabled(bool enabled);
    void setLogFile(const std::filesystem::path& path);

    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    void write(LogLevel level, const std::string& message);

    std::mutex mutex_;
    bool enabled_ = true;
    std::filesystem::path path_{"data/rosterflow.log"};
};

const char* toString(LogLevel level);

} // namespace rosterflow

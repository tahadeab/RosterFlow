#include "logger.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace rosterflow {

const char* toString(LogLevel level) {
    switch (level) {
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enabled;
}

void Logger::setLogFile(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    path_ = path;
}

void Logger::info(const std::string& message) { write(LogLevel::Info, message); }
void Logger::warn(const std::string& message) { write(LogLevel::Warn, message); }
void Logger::error(const std::string& message) { write(LogLevel::Error, message); }

void Logger::write(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!enabled_) return;

    const auto now = std::chrono::system_clock::now();
    const std::time_t seconds = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif

    std::ostringstream stamp;
    stamp << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");

    std::ofstream out(path_, std::ios::app);
    if (!out) {
        static bool warned = false;
        if (!warned) {
            warned = true;
            std::cerr << "Warning: audit log unavailable (" << path_.string()
                      << "); continuing without logging.\n";
        }
        return;
    }
    out << "[" << stamp.str() << "] [" << toString(level) << "] " << message << "\n";
}

} // namespace rosterflow

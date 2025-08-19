#pragma once

#include <mutex>
#include <string>

enum class LogLevel {
    DEBUG, 
    INFO, 
    WARNING, 
    ERROR, 
    CRITICAL
};

class Logger {
public:
    static Logger& get_instance();
    void set_level(const LogLevel& lvl);
    void log(LogLevel lvl, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
private:
    Logger();
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    LogLevel min_level;
    std::string level_to_string(const LogLevel& lvl) const;
    std::string format_log_entry(LogLevel level, const std::string& message);
    std::mutex mtx;
};

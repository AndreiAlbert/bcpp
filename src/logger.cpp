#include "../include/logger.hpp"
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

Logger::Logger(): min_level(LogLevel::DEBUG) {}

std::string Logger::level_to_string(const LogLevel& lvl) const {
    switch (lvl) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
             return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKOWN";
    }
}

Logger& Logger::get_instance() {
    static Logger instance;
    return instance;
}

void Logger::set_level(const LogLevel& lvl) {
    std::lock_guard<std::mutex> lock(mtx);
    min_level = lvl; 
}

std::string Logger::format_log_entry(LogLevel level, const std::string& message) {

    auto now = std::time(nullptr);
    auto local_time = *std::localtime(&now);
    std::stringstream ss; 
    ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S:");
    std::string timestamp = ss.str();
    return std::format("[{}] [{}] {}", timestamp, level_to_string(level), message);
}

void Logger::log(LogLevel lvl, const std::string& message) {
    if (lvl < min_level) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << format_log_entry(lvl, message) << std::endl;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}


void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

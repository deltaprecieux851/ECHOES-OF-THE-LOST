#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>

namespace echoes::core {

namespace {

const char* LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

}  // namespace

void Logger::Log(LogLevel level, std::string_view message) {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::cout << "[" << std::put_time(&localTime, "%H:%M:%S") << "] "
              << "[" << LevelToString(level) << "] "
              << message << std::endl;
}

}  // namespace echoes::core

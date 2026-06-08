#include "core/Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

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

std::filesystem::path GetLogFilePath() {
#ifdef _WIN32
    wchar_t exePath[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) > 0) {
        std::filesystem::path path(exePath);
        path.replace_filename("EchoesOfTheLost.log");
        return path;
    }
#endif
    return std::filesystem::current_path() / "EchoesOfTheLost.log";
}

std::ofstream& GetLogStream() {
    static std::ofstream stream;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        const auto path = GetLogFilePath();
        stream.open(path, std::ios::app);
    }
    return stream;
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


    std::ostringstream formatted;
    formatted << "[" << std::put_time(&localTime, "%H:%M:%S") << "] "
              << "[" << LevelToString(level) << "] "
              << message << std::endl;

    const auto text = formatted.str();
    std::cout << text;

    auto& logStream = GetLogStream();
    if (logStream.is_open()) {
        logStream << text;
        logStream.flush();
    }
}

}  // namespace echoes::core

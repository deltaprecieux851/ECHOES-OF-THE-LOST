#pragma once

#include <string>
#include <string_view>

namespace echoes::core {

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    static void Log(LogLevel level, std::string_view message);
};

}  // namespace echoes::core

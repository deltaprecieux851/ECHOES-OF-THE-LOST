#include "core/Application.h"

#include <windows.h>

#include <shellapi.h>
#include <string>
#include <vector>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::vector<std::string> args;
    std::vector<char*> argv;
    args.reserve(static_cast<std::size_t>(argc));
    argv.reserve(static_cast<std::size_t>(argc));

    for (int i = 0; i < argc; ++i) {
        const int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        args.emplace_back(static_cast<std::size_t>(size > 0 ? size - 1 : 0), '\0');
        if (size > 1) {
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, args.back().data(), size, nullptr, nullptr);
        }
        argv.push_back(args.back().data());
    }

    LocalFree(argvW);

    echoes::core::Application app;
    return app.Run(argc, argv.data());
}

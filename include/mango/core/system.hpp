/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2024 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <string>
#include <mango/core/configure.hpp>
#include <mango/core/thread.hpp>
#include <mango/core/timer.hpp>
#include <mango/core/string.hpp>

namespace mango
{

    struct Context
    {
        mutable ThreadPool thread_pool;
        Timer timer;
        bool debug_print_enable;

        Context();
        ~Context();
    };

    const Context& getSystemContext();

    std::string getPlatformInfo();
    std::string getSystemInfo();

    bool debugPrintIsEnable();
    void debugPrintEnable(bool enable);
    void debugPrint(const char* format, ...);
    void debugPrintLine(const char* format, ...);
    void debugPrintLine(const std::string& text);

    // TODO: testing...

    template <typename... T>
    void __print(T... s)
    {
        fmt::print(s...);
    }

} // namespace mango

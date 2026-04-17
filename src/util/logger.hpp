/*
Copyright (c) Anthony Beaumont
This source code is licensed under the MIT License
found in the LICENSE file in the root directory of this source tree.
*/

#pragma once
#include <fstream>
#include <mutex>
#include <iostream>
#include <format>

class Logger {
public:
    enum class Level {
        info = 0,
        warn,
        error
    };
    
private:
    std::ofstream file;
    std::mutex mtx;
    Level minLevel = Level::info;
    
    std::string lvlToStr(Level level) {
        switch (level) {
            case Level::info:  return "INFO";
            case Level::warn:  return "WARN";
            case Level::error: return "ERROR";
            default:           return "INFO";
        }
    }
    
    template<typename... Args> void log(Level level, std::format_string<Args...> msg, Args&&... args) {
        if (level < minLevel) return;
        
        std::string formatted = std::format(msg, std::forward<Args>(args)...);
        std::string message = "[" + lvlToStr(level) + "] " + formatted;

        std::lock_guard<std::mutex> lock(mtx);
        std::cout << message << std::endl;
        if (file.is_open()) {
            file << message << std::endl;
        }
    }
    
public:
    Logger() = default;
    
    Logger(const std::string& filename){
        file.open(filename, std::ios::trunc);
          if (!file.is_open()) {
              std::cerr << "Failed to open log file" << std::endl;
        }
    }
    
    void setLogLevel(Level level) {
        std::lock_guard<std::mutex> lock(mtx);
        minLevel = level;
    }

    template<typename... Args> void log(std::format_string<Args...> msg, Args&&... args) {
        log(Level::info, msg, std::forward<Args>(args)...);
    }

    template<typename... Args> void info(std::format_string<Args...> msg, Args&&... args) {
        log(Level::info, msg, std::forward<Args>(args)...);
    }

    template<typename... Args> void warn(std::format_string<Args...> msg, Args&&... args) {
        log(Level::warn, msg, std::forward<Args>(args)...);
    }

    template<typename... Args> void error(std::format_string<Args...> msg, Args&&... args) {
        log(Level::error, msg, std::forward<Args>(args)...);
    }
};
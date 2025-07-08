#pragma once

#include "Config.h"
#include <string>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>

// Kelas buat ngehandle logging
class Logger {
private:
    // Mutex buat sinkronisasi akses ke log
    static std::mutex logMutex;
    // File log yang lagi dibuka
    static std::ofstream logFile;
    // Flag apakah logger udah diinisialisasi
    static bool initialized;
    
    // Dapetin waktu sekarang dalam format string
    static std::string getCurrentTimeString() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
        return ss.str();
    }
    
    // Inisialisasi logger kalo belom
    static void initialize() {
        if (!initialized) {
            logFile.open("infernobot.log", std::ios::app);
            initialized = true;
        }
    }
    
    // Tulis log dengan level tertentu
    template<typename T>
    static void log(Config::LogLevel level, const T& message) {
        // Skip kalo level log lebih rendah dari yang diset
        if (level < Config::LOG_LEVEL) {
            return;
        }
        
        // Konversi level log ke string
        std::string levelStr;
        switch(level) {
            case Config::LogLevel::DEBUG:
                levelStr = "DEBUG";
                break;
            case Config::LogLevel::INFO:
                levelStr = "INFO";
                break;
            case Config::LogLevel::WARNING:
                levelStr = "WARNING";
                break;
            case Config::LogLevel::ERROR:
                levelStr = "ERROR";
                break;
            case Config::LogLevel::CRITICAL:
                levelStr = "CRITICAL";
                break;
        }
        
        std::string timestamp = getCurrentTimeString();
        
        // Lock mutex biar ga bentrok pas nulis log
        std::lock_guard<std::mutex> lock(logMutex);
        initialize();
        
        // Format output log
        std::stringstream output;
        output << timestamp << " - " << levelStr << " - " << message << std::endl;
        
        // Tulis ke console dan file
        std::cout << output.str();
        if (logFile.is_open()) {
            logFile << output.str();
            logFile.flush();
        }
    }
    
public:
    // Log level DEBUG
    template<typename T>
    static void debug(const T& message) {
        log(Config::LogLevel::DEBUG, message);
    }
    
    // Log level INFO
    template<typename T>
    static void info(const T& message) {
        log(Config::LogLevel::INFO, message);
    }
    
    // Log level WARNING
    template<typename T>
    static void warning(const T& message) {
        log(Config::LogLevel::WARNING, message);
    }
    
    // Log level ERROR
    template<typename T>
    static void error(const T& message) {
        log(Config::LogLevel::ERROR, message);
    }
    
    // Log level CRITICAL
    template<typename T>
    static void critical(const T& message) {
        log(Config::LogLevel::CRITICAL, message);
    }
}; 
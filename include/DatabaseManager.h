#pragma once

#include "User.h"
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <chrono>
#include <optional>
#include <functional>
#include <thread>
#include <atomic>

// Kelas buat ngatur database dan file cache
class DatabaseManager {
private:
    // Singleton instance
    static DatabaseManager* instance;
    static std::mutex instanceMutex;
    // Cache buat user terdaftar
    std::vector<EnhancedUser> usersCache;
    // Cache buat user ga terdaftar
    std::vector<UnregisteredUser> unregisteredCache;
    // Waktu terakhir update cache
    std::chrono::system_clock::time_point lastCacheUpdate;
    std::mutex cacheMutex;
    // Struktur buat job menulis ke database
    struct WriteJob {
        std::optional<std::vector<EnhancedUser>> users;
        std::optional<std::vector<UnregisteredUser>> unregisteredUsers;
    };
    // Antrian job yang mau ditulis ke database
    std::queue<WriteJob> writeQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    // Thread buat nulis database di background
    std::thread writeThread;
    std::atomic<bool> shouldStop;
    // Path ke direktori database
    std::string directoryPath;
    // Path file database user
    std::string userDbFile;
    // Path file database user ga terdaftar
    std::string unregisteredDbFile;
    // Constructor private buat singleton
    DatabaseManager();
    ~DatabaseManager();
    // Inisialisasi database kalo belom ada
    void initDatabases();
    // Worker buat nulis database di background
    void writeWorker();
    
public:
    // Dapetin instance singleton
    static DatabaseManager& getInstance();
    // Ga boleh di-copy
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    // Load daftar user terdaftar
    std::vector<EnhancedUser> loadUsers();
    // Load daftar user ga terdaftar
    std::vector<UnregisteredUser> loadUnregisteredUsers();
    // Simpan daftar user terdaftar (langsung)
    void saveUsers(const std::vector<EnhancedUser>& users);
    // Simpan daftar user ga terdaftar (langsung)
    void saveUnregisteredUsers(const std::vector<UnregisteredUser>& users);
    // Antri job nyimpan user terdaftar (async)
    void queueSaveUsers(const std::vector<EnhancedUser>& users);
    // Antri job nyimpan user ga terdaftar (async)
    void queueSaveUnregisteredUsers(const std::vector<UnregisteredUser>& users);
    // Set path direktori database
    void setDirectoryPath(const std::string& path);
    // Dapetin path direktori database
    std::string getDirectoryPath() const { return directoryPath; }
    
    // Kelas buat cache file database
    class FileCache {
    private:
        // Daftar file yang di-cache
        static std::vector<std::string> files;
        // Waktu terakhir update cache
        static std::chrono::system_clock::time_point lastUpdated;
        // Interval refresh cache (detik)
        static const int refreshInterval = 300;
        static std::mutex fileCacheMutex;
        
    public:
        // Dapetin daftar file .txt di direktori database
        static std::vector<std::string> getTxtFiles();
    };
}; 
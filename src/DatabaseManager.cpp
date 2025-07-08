#include "../include/DatabaseManager.h"
#include "../include/Config.h"
#include "../include/Logger.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <thread>
DatabaseManager* DatabaseManager::instance = nullptr;
std::mutex DatabaseManager::instanceMutex;
std::vector<std::string> DatabaseManager::FileCache::files;
std::chrono::system_clock::time_point DatabaseManager::FileCache::lastUpdated;
std::mutex DatabaseManager::FileCache::fileCacheMutex;
DatabaseManager::DatabaseManager() : 
    lastCacheUpdate(std::chrono::system_clock::now()),
    shouldStop(false),
    directoryPath(Config::DIRECTORY_PATH) {
    userDbFile = directoryPath + "users.json";
    unregisteredDbFile = directoryPath + "unregistered_users.json";
    initDatabases();
    writeThread = std::thread(&DatabaseManager::writeWorker, this);
}
DatabaseManager::~DatabaseManager() {
    shouldStop = true;
    queueCV.notify_all();
    if (writeThread.joinable()) {
        writeThread.join();
    }
}

DatabaseManager& DatabaseManager::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new DatabaseManager();
    }
    return *instance;
}

void DatabaseManager::initDatabases() {
    try {
        std::filesystem::path dir(directoryPath);
        if (!std::filesystem::exists(dir)) {
            Logger::info("Creating directory: " + directoryPath);
            std::filesystem::create_directories(dir);
        }
        if (!std::filesystem::exists(userDbFile)) {
            std::ofstream file(userDbFile);
            if (file.is_open()) {
                json data = {
                    {"version", "2.1"},
                    {"metadata", {
                        {"last_updated", Utils::getCurrentIsoDateTime()},
                        {"total_users", 0},
                        {"active_users", 0}
                    }},
                    {"users", json::array()}
                };
                file << data.dump(2);
                file.close();
            } else {
                Logger::error("Failed to create user database file: " + userDbFile);
            }
        }
        if (!std::filesystem::exists(unregisteredDbFile)) {
            std::ofstream file(unregisteredDbFile);
            if (file.is_open()) {
                json data = {
                    {"version", "2.1"},
                    {"metadata", {
                        {"last_updated", Utils::getCurrentIsoDateTime()},
                        {"total_unregistered", 0},
                        {"active_unregistered", 0}
                    }},
                    {"users", json::array()}
                };
                file << data.dump(2);
                file.close();
            } else {
                Logger::error("Failed to create unregistered user database file: " + unregisteredDbFile);
            }
        }
    } catch (const std::exception& e) {
        Logger::error("Error initializing databases: " + std::string(e.what()));
    }
}

std::vector<EnhancedUser> DatabaseManager::loadUsers() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCacheUpdate).count();
    if (!usersCache.empty() && elapsed < 300) {
        return usersCache;
    }
    try {
        std::ifstream file(userDbFile);
        if (!file.is_open()) {
            Logger::error("Failed to open user database file: " + userDbFile);
            return {};
        }
        json data = json::parse(file);
        file.close();
        
        std::vector<EnhancedUser> users;
        for (const auto& userJson : data["users"]) {
            users.push_back(EnhancedUser::fromJson(userJson));
        }
        usersCache = users;
        lastCacheUpdate = now;
        return users;
    } catch (const std::exception& e) {
        Logger::error("Error loading users: " + std::string(e.what()));
        return {};
    }
}

std::vector<UnregisteredUser> DatabaseManager::loadUnregisteredUsers() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCacheUpdate).count();
    if (!unregisteredCache.empty() && elapsed < 300) {
        return unregisteredCache;
    }
    try {
        std::ifstream file(unregisteredDbFile);
        if (!file.is_open()) {
            Logger::error("Failed to open unregistered user database file: " + unregisteredDbFile);
            return {};
        }
        json data = json::parse(file);
        file.close();
        std::vector<UnregisteredUser> users;
        for (const auto& userJson : data["users"]) {
            users.push_back(UnregisteredUser::fromJson(userJson));
        }
        unregisteredCache = users;
        lastCacheUpdate = now;
        return users;
    } catch (const std::exception& e) {
        Logger::error("Error loading unregistered users: " + std::string(e.what()));
        return {};
    }
}

void DatabaseManager::saveUsers(const std::vector<EnhancedUser>& users) {
    try {
        int activeCount = 0;
        for (const auto& user : users) {
            if (user.isActive()) {
                activeCount++;
            }
        }
        
        json userData = json::array();
        for (const auto& user : users) {
            userData.push_back(user.toJson());
        }
        
        json data = {
            {"version", "2.1"},
            {"metadata", {
                {"last_updated", Utils::getCurrentIsoDateTime()},
                {"total_users", users.size()},
                {"active_users", activeCount}
            }},
            {"users", userData}
        };
        
        std::ofstream file(userDbFile);
        if (!file.is_open()) {
            Logger::error("Failed to open user database file for writing: " + userDbFile);
            return;
        }
        
        file << data.dump(2);
        file.close();
        
        
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            usersCache = users;
            lastCacheUpdate = std::chrono::system_clock::now();
        }
        
        Logger::info("Saved " + std::to_string(users.size()) + " users to database");
    } catch (const std::exception& e) {
        Logger::error("Error saving users: " + std::string(e.what()));
    }
}

void DatabaseManager::saveUnregisteredUsers(const std::vector<UnregisteredUser>& users) {
    try {
        int activeCount = 0;
        for (const auto& user : users) {
            if (!user.isExhausted() && user.getTokens() > 0) {
                activeCount++;
            }
        }
        
        json userData = json::array();
        for (const auto& user : users) {
            userData.push_back(user.toJson());
        }
        
        json data = {
            {"version", "2.1"},
            {"metadata", {
                {"last_updated", Utils::getCurrentIsoDateTime()},
                {"total_unregistered", users.size()},
                {"active_unregistered", activeCount}
            }},
            {"users", userData}
        };
        
        std::ofstream file(unregisteredDbFile);
        if (!file.is_open()) {
            Logger::error("Failed to open unregistered user database file for writing: " + unregisteredDbFile);
            return;
        }
        
        file << data.dump(2);
        file.close();
        
        
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            unregisteredCache = users;
            lastCacheUpdate = std::chrono::system_clock::now();
        }
        
        Logger::info("Saved " + std::to_string(users.size()) + " unregistered users to database");
    } catch (const std::exception& e) {
        Logger::error("Error saving unregistered users: " + std::string(e.what()));
    }
}

void DatabaseManager::queueSaveUsers(const std::vector<EnhancedUser>& users) {
    WriteJob job;
    job.users = users;
    
    std::lock_guard<std::mutex> lock(queueMutex);
    writeQueue.push(job);
    queueCV.notify_one();
}

void DatabaseManager::queueSaveUnregisteredUsers(const std::vector<UnregisteredUser>& users) {
    WriteJob job;
    job.unregisteredUsers = users;
    
    std::lock_guard<std::mutex> lock(queueMutex);
    writeQueue.push(job);
    queueCV.notify_one();
}

void DatabaseManager::writeWorker() {
    while (!shouldStop) {
        WriteJob job;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait_for(lock, std::chrono::seconds(30), [this] {
                return !writeQueue.empty() || shouldStop;
            });
            
            if (shouldStop) {
                break;
            }
            
            if (!writeQueue.empty()) {
                job = writeQueue.front();
                writeQueue.pop();
            } else {
                continue;
            }
        }
        
        if (job.users.has_value()) {
            saveUsers(job.users.value());
        }
        
        if (job.unregisteredUsers.has_value()) {
            saveUnregisteredUsers(job.unregisteredUsers.value());
        }
    }
    
    Logger::info("Write worker thread stopped");
}

void DatabaseManager::setDirectoryPath(const std::string& path) {
    directoryPath = path;
    if (directoryPath.back() != '/') {
        directoryPath += '/';
    }
    
    userDbFile = directoryPath + "users.json";
    unregisteredDbFile = directoryPath + "unregistered_users.json";
}

std::vector<std::string> DatabaseManager::FileCache::getTxtFiles() {
    std::lock_guard<std::mutex> lock(fileCacheMutex);
    
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdated).count();
    
    if (!files.empty() && elapsed < refreshInterval) {
        return files;
    }
    
    try {
        std::string dirPath = DatabaseManager::getInstance().getDirectoryPath();
        std::vector<std::string> result;
        
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.size() >= 4) {
                    std::string extension = filename.substr(filename.size() - 4);
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    if (extension == ".txt") {
                        result.push_back(filename);
                    }
                }
            }
        }
        
        files = result;
        lastUpdated = now;
        return files;
    } catch (const std::exception& e) {
        Logger::error("Error updating file cache: " + std::string(e.what()));
        return {};
    }
} 
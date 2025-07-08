#include "../include/User.h"
#include "../include/Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Implementasi EnhancedUser nih bro
EnhancedUser::EnhancedUser(
    int64_t userId,
    const std::string& username,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& expiryDate,
    int duration,
    const std::string& unit,
    const std::string& status,
    const std::string& adminLevel
) : userId(userId),
    username(username),
    firstName(firstName),
    lastName(lastName),
    expiryDate(expiryDate),
    duration(duration),
    unit(unit),
    status(status),
    totalSearches(0),
    lastActivity(""),
    concurrentRequests(0),
    adminLevel(adminLevel) {
    
    // Set waktu aktivitas terakhir ke waktu sekarang coy
    updateLastActivity();
}

bool EnhancedUser::isActive() const {
    if (status != "active") {
        return false;
    }
    
    try {
        // Parse tanggal kedaluwarsa dulu gan
        std::tm tm = {};
        std::istringstream ss(expiryDate);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            return false;
        }
        
        auto expiryTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        auto now = std::chrono::system_clock::now();
        
        return now < expiryTime;
    } catch (const std::exception& e) {
        Logger::warning("Error parsing expiry date: " + std::string(e.what()));
        return false;
    }
}

bool EnhancedUser::canMakeRequest() const {
    return concurrentRequests < MAX_CONCURRENT_REQUESTS;
}

void EnhancedUser::incrementTotalSearches() {
    totalSearches++;
}

void EnhancedUser::incrementConcurrentRequests() {
    concurrentRequests++;
}

void EnhancedUser::decrementConcurrentRequests() {
    if (concurrentRequests > 0) {
        concurrentRequests--;
    }
}

void EnhancedUser::updateLastActivity() {
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&nowTime), "%Y-%m-%dT%H:%M:%S") << "Z";
    lastActivity = ss.str();
}

json EnhancedUser::toJson() const {
    return {
        {"id", userId},
        {"username", username},
        {"first_name", firstName},
        {"last_name", lastName},
        {"expiry", expiryDate},
        {"duration", duration},
        {"unit", unit},
        {"status", status},
        {"total_searches", totalSearches},
        {"last_activity", lastActivity},
        {"admin_level", adminLevel}
    };
}

EnhancedUser EnhancedUser::fromJson(const json& data) {
    EnhancedUser user(
        data["id"].get<int64_t>(),
        data.value("username", ""),
        data.value("first_name", ""),
        data.value("last_name", ""),
        data["expiry"].get<std::string>(),
        data["duration"].get<int>(),
        data["unit"].get<std::string>(),
        data.value("status", "active"),
        data.value("admin_level", "none")
    );
    
    user.totalSearches = data.value("total_searches", 0);
    user.lastActivity = data.value("last_activity", "");
    
    return user;
}

// Implementasi UnregisteredUser nih bro
UnregisteredUser::UnregisteredUser(
    int64_t userId,
    const std::string& username,
    const std::string& firstName,
    const std::string& lastName
) : userId(userId),
    username(username),
    firstName(firstName),
    lastName(lastName),
    tokens(FREE_CREDIT_DAILY),
    lastReset(std::chrono::system_clock::now()),
    exhausted(false),
    concurrentRequests(0),
    permanentlyBlocked(false) {
    
    // Set waktu aktivitas terakhir ke waktu sekarang coy
    updateLastActivity();
}

bool UnregisteredUser::checkAndUpdateTokens() {
    if (permanentlyBlocked) {
        return false;
    }
    
    if (exhausted) {
        return false;
    }
    
    if (tokens <= 0) {
        exhausted = true;
        permanentlyBlocked = true;
        return false;
    }
    
    tokens--;
    updateLastActivity();
    return true;
}

bool UnregisteredUser::canMakeRequest() {
    return concurrentRequests < MAX_CONCURRENT_REQUESTS && ((!exhausted) || checkAndUpdateTokens());
}

void UnregisteredUser::incrementConcurrentRequests() {
    concurrentRequests++;
}

void UnregisteredUser::decrementConcurrentRequests() {
    if (concurrentRequests > 0) {
        concurrentRequests--;
    }
}

void UnregisteredUser::updateLastActivity() {
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&nowTime), "%Y-%m-%dT%H:%M:%S") << "Z";
    lastActivity = ss.str();
}

json UnregisteredUser::toJson() const {
    // Ubah lastReset ke string ISO dulu bro
    auto time = std::chrono::system_clock::to_time_t(lastReset);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S") << "Z";
    
    return {
        {"user_id", userId},
        {"username", username},
        {"first_name", firstName},
        {"last_name", lastName},
        {"tokens", tokens},
        {"last_reset", ss.str()},
        {"exhausted", exhausted},
        {"last_activity", lastActivity},
        {"permanently_blocked", permanentlyBlocked}
    };
}

UnregisteredUser UnregisteredUser::fromJson(const json& data) {
    UnregisteredUser user(
        data["user_id"].get<int64_t>(),
        data.value("username", ""),
        data.value("first_name", ""),
        data.value("last_name", "")
    );
    
    user.tokens = data["tokens"].get<int>();
    
    // Parse tanggal last_reset dulu gan
    std::tm tm = {};
    std::string lastResetStr = data["last_reset"].get<std::string>();
    std::istringstream ss(lastResetStr);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (!ss.fail()) {
        user.lastReset = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }
    
    user.exhausted = data["exhausted"].get<bool>();
    user.lastActivity = data.value("last_activity", "");
    user.permanentlyBlocked = data.value("permanently_blocked", false);
    
    return user;
} 
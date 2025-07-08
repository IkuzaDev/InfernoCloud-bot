#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
// Maksimal request bersamaan per user
constexpr int MAX_CONCURRENT_REQUESTS = 3;
// Kredit gratis harian buat user ga terdaftar
constexpr int FREE_CREDIT_DAILY = 5;

using json = nlohmann::json;

// Kelas buat user premium yang udah terdaftar
class EnhancedUser {
private:
    int64_t userId;
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string expiryDate;
    int duration;
    std::string unit;
    std::string status;
    int totalSearches;
    std::string lastActivity;
    int concurrentRequests;
    std::string adminLevel;

public:
    EnhancedUser(
        int64_t userId = 0,
        const std::string& username = "",
        const std::string& firstName = "",
        const std::string& lastName = "",
        const std::string& expiryDate = "",
        int duration = 0,
        const std::string& unit = "day",
        const std::string& status = "active",
        const std::string& adminLevel = "none"
    );
    int64_t getUserId() const { return userId; }
    std::string getUsername() const { return username; }
    std::string getFirstName() const { return firstName; }
    std::string getLastName() const { return lastName; }
    std::string getExpiryDate() const { return expiryDate; }
    int getDuration() const { return duration; }
    std::string getUnit() const { return unit; }
    std::string getStatus() const { return status; }
    int getTotalSearches() const { return totalSearches; }
    std::string getLastActivity() const { return lastActivity; }
    int getConcurrentRequests() const { return concurrentRequests; }
    std::string getAdminLevel() const { return adminLevel; }
    
    void setUsername(const std::string& value) { username = value; }
    void setFirstName(const std::string& value) { firstName = value; }
    void setLastName(const std::string& value) { lastName = value; }
    void setExpiryDate(const std::string& value) { expiryDate = value; }
    void setDuration(int value) { duration = value; }
    void setUnit(const std::string& value) { unit = value; }
    void setStatus(const std::string& value) { status = value; }
    void setTotalSearches(int value) { totalSearches = value; }
    void setLastActivity(const std::string& value) { lastActivity = value; }
    void setConcurrentRequests(int value) { concurrentRequests = value; }
    void setAdminLevel(const std::string& value) { adminLevel = value; }

    // Cek apakah user masih aktif (belom expired)
    bool isActive() const;
    // Cek apakah user bisa bikin request baru
    bool canMakeRequest() const;
    // Nambah total pencarian user
    void incrementTotalSearches();
    // Nambah jumlah request bersamaan
    void incrementConcurrentRequests();
    // Kurangin jumlah request bersamaan
    void decrementConcurrentRequests();
    // Update waktu aktivitas terakhir
    void updateLastActivity();
    // Ubah ke format JSON
    json toJson() const;
    // Bikin objek dari JSON
    static EnhancedUser fromJson(const json& data);
};

// Kelas buat user yang belom terdaftar (pake kredit gratis)
class UnregisteredUser {
private:
    int64_t userId;
    std::string username;
    std::string firstName;
    std::string lastName;
    int tokens;
    std::chrono::system_clock::time_point lastReset;
    bool exhausted;
    std::string lastActivity;
    int concurrentRequests;
    bool permanentlyBlocked;

public:
    UnregisteredUser(
        int64_t userId = 0,
        const std::string& username = "",
        const std::string& firstName = "",
        const std::string& lastName = ""
    );
    int64_t getUserId() const { return userId; }
    std::string getUsername() const { return username; }
    std::string getFirstName() const { return firstName; }
    std::string getLastName() const { return lastName; }
    int getTokens() const { return tokens; }
    std::chrono::system_clock::time_point getLastReset() const { return lastReset; }
    bool isExhausted() const { return exhausted; }
    std::string getLastActivity() const { return lastActivity; }
    int getConcurrentRequests() const { return concurrentRequests; }
    bool isPermanentlyBlocked() const { return permanentlyBlocked; }
    
    void setUsername(const std::string& value) { username = value; }
    void setFirstName(const std::string& value) { firstName = value; }
    void setLastName(const std::string& value) { lastName = value; }
    void setTokens(int value) { tokens = value; }
    void setLastReset(const std::chrono::system_clock::time_point& value) { lastReset = value; }
    void setExhausted(bool value) { exhausted = value; }
    void setLastActivity(const std::string& value) { lastActivity = value; }
    void setConcurrentRequests(int value) { concurrentRequests = value; }
    void setPermanentlyBlocked(bool value) { permanentlyBlocked = value; }
    
    // Cek dan update token user, kalo abis jadi exhausted
    bool checkAndUpdateTokens();
    // Cek apakah user bisa bikin request baru
    bool canMakeRequest();
    // Nambah jumlah request bersamaan
    void incrementConcurrentRequests();
    // Kurangin jumlah request bersamaan
    void decrementConcurrentRequests();
    // Update waktu aktivitas terakhir
    void updateLastActivity();
    
    // Ubah ke format JSON
    json toJson() const;
    // Bikin objek dari JSON
    static UnregisteredUser fromJson(const json& data);
}; 
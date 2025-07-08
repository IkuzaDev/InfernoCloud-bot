#pragma once

#include "User.h"
#include <memory>
#include <utility>
#include <optional>
#include <mutex>
#include <tgbot/tgbot.h>

// Kelas buat ngatur user dan izin akses
class UserManager {
private:
    // Mutex buat sinkronisasi akses ke user
    static std::mutex userManagementMutex;

public:
    // Dapetin atau bikin user baru dari update Telegram
    static std::pair<std::optional<EnhancedUser>, std::optional<UnregisteredUser>> 
    getOrCreateUser(const TgBot::Update& update);

    // Cek apakah user boleh pake perintah tertentu
    static bool isUserAllowed(int64_t userId, const std::string& command);
    
    // Lepas request user setelah selesai
    static void releaseUserRequest(int64_t userId);
    
    // Handle kalo user ga punya token/kredit
    static void handleNoTokens(const TgBot::Update& update, const std::string& command);
};

// Kelas buat fungsi-fungsi utility
class Utils {
public:
    // Dapetin waktu sekarang dalam format ISO
    static std::string getCurrentIsoDateTime();
    // Parse string ISO jadi time_point
    static std::chrono::system_clock::time_point parseIsoDateTime(const std::string& iso);
    // Format time_point jadi string dengan format tertentu
    static std::string formatDateTime(const std::chrono::system_clock::time_point& dt, const std::string& format = "%Y-%m-%d %H:%M:%S");
    // Hitung tanggal kadaluarsa berdasarkan durasi
    static std::chrono::system_clock::time_point calculateExpiry(int duration, const std::string& unit);
    
    // Bersihin nama file dari karakter yang ga valid
    static std::string sanitizeFilename(const std::string& input);
    // Filter baris yang ga diinginkan dari hasil search
    static bool filterUnwantedLine(const std::string& line);
}; 
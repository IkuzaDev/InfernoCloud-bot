#pragma once

#include <tgbot/tgbot.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <future>

// Kelas buat handle semua perintah bot
class CommandHandlers {
private:
    // Mutex buat sinkronisasi akses
    static std::mutex busyLock;
    // Set perintah yang lagi ditutup
    static std::unordered_set<std::string> closedCommands;
    // Map request yang lagi aktif
    static std::map<std::string, std::pair<int64_t, std::chrono::system_clock::time_point>> activeRequests;

public:
    // Daftarin semua handler perintah ke bot
    static void registerHandlers(TgBot::Bot& bot);
    // Setup job yang bakal jalan terus-terusan
    static void setupRepeatingJobs(TgBot::Bot& bot);
    // Handler perintah /start
    static void start(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /menu
    static void menu(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /mytokens
    static void myTokens(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /close (tutup perintah)
    static void closeCommand(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /closed (liat perintah yang ditutup)
    static void closedCommand(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /add_admin (tambah admin)
    static void addAdmin(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /ban_admin (ban admin)
    static void banAdmin(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /add (tambah user)
    static void addUser(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /extend (perpanjang user)
    static void extendUser(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /dell (hapus user)
    static void dellUser(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /listuser (daftar user)
    static void listUsers(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /usertotal (jumlah user)
    static void userTotal(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /dbtotal (jumlah database)
    static void dbTotal(const TgBot::Message::Ptr& message, TgBot::Bot& bot);
    // Handler perintah /search (cari keyword)
    static void searchDomain(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler perintah /weblist (list website)
    static void weblist(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
    // Handler buat callback query dari inline keyboard
    static void callbackQueryHandler(const TgBot::CallbackQuery::Ptr& query, TgBot::Bot& bot);
    
    // Cek user yang udah expired
    static void checkExpiryUsers(TgBot::Bot& bot);
    // Cek expired buat satu user
    static void checkExpiry(TgBot::Bot& bot, int64_t userId);
    // Auto delete user yang udah expired
    static void autoDeleteUser(TgBot::Bot& bot, int64_t userId);
    // Bersihin request yang udah timeout
    static void cleanupActiveRequests(TgBot::Bot& bot);
    // Jaga bot tetep hidup
    static void keepAlive(TgBot::Bot& bot);

private:
    // Proses request pencarian keyword
    static void processSearchRequest(
        const TgBot::Message::Ptr& message, 
        TgBot::Bot& bot, 
        const std::string& keyword, 
        int initialMessageId, 
        const std::string& requestId
    );
    
    // Proses request weblist
    static void processWeblistRequest(
        const TgBot::Message::Ptr& message, 
        TgBot::Bot& bot, 
        const std::string& tld, 
        const std::string& requestId
    );

    // Cari keyword dalam satu file
    static void searchFile(
        const std::string& filePath, 
        const std::string& keyword,
        std::vector<std::string>& results,
        std::mutex& resultsMutex,
        std::atomic<bool>& stopFlag
    );
    
    // Ekstrak URL dari file berdasarkan TLD
    static void extractUrlsFromFile(
        const std::string& filePath,
        const std::string& tld,
        std::unordered_set<std::string>& foundUrls,
        std::mutex& urlsMutex,
        std::atomic<bool>& stopFlag
    );
}; 
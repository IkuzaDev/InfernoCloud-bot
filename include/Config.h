#pragma once

#include <string>
#include <unordered_set>
#include <chrono>
namespace Config {
    // Token bot lu dapetin dari @BotFather
    inline const std::string TOKEN = "7744721958:AAGd9zWdKPOJEYmQ58RIybWNnu2sO3hheak";
    // ID admin master buat ngontrol semuanya
    inline const std::unordered_set<int64_t> MASTER_ADMIN_IDS = {8023711444};
    // ID admin level pertama
    inline const std::unordered_set<int64_t> FIRST_ADMIN_IDS = {1659159850};
    // ID admin level kedua
    inline const std::unordered_set<int64_t> SECOND_ADMIN_IDS = {7407179259};
    // Kontak admin buat user nanya-nanya
    inline const std::string ADMIN_CONTACT = "@avxxr00t";
    // Path ke direktori database
    inline const std::string DIRECTORY_PATH = "/home/ubuntu/inferno_c2/inferno_cloud/";
    // Jumlah worker thread buat search
    inline const int WORKERS = 60;
    // Berapa jam sebelum token di-reset
    inline const int TOKEN_RESET_HOURS = 24;
    // Timeout request dalam detik
    inline const int REQUEST_TIMEOUT = 300;
    // Maksimal hasil pencarian yang dikembaliin
    inline const int MAX_SEARCH_RESULTS = 1200;
    // Maksimal hasil weblist yang dikembaliin
    inline const int MAX_WEBLIST_RESULTS = 50000;
    // Maksimal request bersamaan per user
    inline const int MAX_CONCURRENT_REQUESTS = 3;
    // Kredit gratis harian buat user ga terdaftar
    inline const int FREE_CREDIT_DAILY = 5;
    // Ukuran batch buat operasi search
    inline const int SEARCH_BATCH_SIZE = 50;
    // Ukuran file maksimal yang bisa di-search (100MB)
    inline const size_t MAX_FILE_SIZE = 100 * 1024 * 1024;
    // Interval refresh cache
    inline const auto CACHE_REFRESH_INTERVAL = std::chrono::seconds(300); 
    // Interval pembersihan request yang aktif
    inline const auto REQUEST_CLEANUP_INTERVAL = std::chrono::hours(1); 
    // Interval keep-alive buat jaga bot tetep jalan
    inline const auto KEEP_ALIVE_INTERVAL = std::chrono::minutes(1); 
    // Interval cek user yang udah expired
    inline const auto EXPIRY_CHECK_INTERVAL = std::chrono::minutes(30); 
    // Delay sebelum auto-delete user
    inline const auto AUTO_DELETE_DELAY = std::chrono::hours(2);
    // Level log yang tersedia
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };
    
    // Level log yang dipake sekarang
    inline const LogLevel LOG_LEVEL = LogLevel::INFO;
} 
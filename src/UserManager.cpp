#include "../include/UserManager.h"
#include "../include/DatabaseManager.h"
#include "../include/Config.h"
#include "../include/Logger.h"

// Inisialisasi variabel statik
std::mutex UserManager::userManagementMutex;

std::pair<std::optional<EnhancedUser>, std::optional<UnregisteredUser>>
UserManager::getOrCreateUser(const TgBot::Update& update) {
    int64_t userId = update.message->from->id;
    std::string username = update.message->from->username.has_value() ? 
                          update.message->from->username.value() : "";
    std::string firstName = update.message->from->firstName.has_value() ? 
                           update.message->from->firstName.value() : "";
    std::string lastName = update.message->from->lastName.has_value() ? 
                          update.message->from->lastName.value() : "";
    
    std::lock_guard<std::mutex> lock(userManagementMutex);
    auto& dbManager = DatabaseManager::getInstance();
    
    // Cek dulu nih user udah terdaftar belom
    auto users = dbManager.loadUsers();
    for (auto& user : users) {
        if (user.getUserId() == userId) {
            // Update info user
            user.setUsername(username);
            user.setFirstName(firstName);
            user.setLastName(lastName);
            user.updateLastActivity();
            
            // Simpen daftar user yang udah diupdate
            dbManager.queueSaveUsers(users);
            
            // Balikin user
            return {user, std::nullopt};
        }
    }
    
    // Cek kalo user ga terdaftar
    auto unregisteredUsers = dbManager.loadUnregisteredUsers();
    for (auto& user : unregisteredUsers) {
        if (user.getUserId() == userId) {
            // Update info user
            user.setUsername(username);
            user.setFirstName(firstName);
            user.setLastName(lastName);
            user.updateLastActivity();
            
            // Simpen daftar user yang udah diupdate
            dbManager.queueSaveUnregisteredUsers(unregisteredUsers);
            
            // Balikin user
            return {std::nullopt, user};
        }
    }
    
    // Bikin user baru yang ga terdaftar
    UnregisteredUser newUser(userId, username, firstName, lastName);
    unregisteredUsers.push_back(newUser);
    dbManager.queueSaveUnregisteredUsers(unregisteredUsers);
    
    // Balikin user baru
    return {std::nullopt, newUser};
}

bool UserManager::isUserAllowed(int64_t userId, const std::string& command) {
    // Cek apakah user ini admin
    if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end() ||
        Config::FIRST_ADMIN_IDS.find(userId) != Config::FIRST_ADMIN_IDS.end() ||
        Config::SECOND_ADMIN_IDS.find(userId) != Config::SECOND_ADMIN_IDS.end()) {
        return true;
    }
    
    // TODO: Implementasiin cek perintah tertutup
    
    std::lock_guard<std::mutex> lock(userManagementMutex);
    auto& dbManager = DatabaseManager::getInstance();
    
    // Cek kalo user terdaftar
    auto users = dbManager.loadUsers();
    for (auto& user : users) {
        if (user.getUserId() == userId && user.isActive()) {
            if (!user.canMakeRequest()) {
                return false;
            }
            
            user.incrementTotalSearches();
            user.incrementConcurrentRequests();
            user.updateLastActivity();
            
            dbManager.queueSaveUsers(users);
            return true;
        }
    }
    
    // Cek kalo user ga terdaftar
    auto unregisteredUsers = dbManager.loadUnregisteredUsers();
    for (auto& user : unregisteredUsers) {
        if (user.getUserId() == userId) {
            if (!user.canMakeRequest()) {
                return false;
            }
            
            user.incrementConcurrentRequests();
            dbManager.queueSaveUnregisteredUsers(unregisteredUsers);
            return true;
        }
    }
    
    return false;
}

void UserManager::releaseUserRequest(int64_t userId) {
    std::lock_guard<std::mutex> lock(userManagementMutex);
    auto& dbManager = DatabaseManager::getInstance();
    
    // Cek kalo user terdaftar
    auto users = dbManager.loadUsers();
    for (auto& user : users) {
        if (user.getUserId() == userId) {
            user.decrementConcurrentRequests();
            dbManager.queueSaveUsers(users);
            return;
        }
    }
    
    // Cek kalo user ga terdaftar
    auto unregisteredUsers = dbManager.loadUnregisteredUsers();
    for (auto& user : unregisteredUsers) {
        if (user.getUserId() == userId) {
            user.decrementConcurrentRequests();
            dbManager.queueSaveUnregisteredUsers(unregisteredUsers);
            return;
        }
    }
}

void UserManager::handleNoTokens(const TgBot::Update& update, const std::string& command) {
    int64_t userId = update.message->from->id;
    
    std::lock_guard<std::mutex> lock(userManagementMutex);
    auto& dbManager = DatabaseManager::getInstance();
    
    // Cek kalo user ga terdaftar
    auto unregisteredUsers = dbManager.loadUnregisteredUsers();
    for (const auto& user : unregisteredUsers) {
        if (user.getUserId() == userId) {
            std::string paymentMessage = 
                "🚫 Anda telah menghabiskan semua " + std::to_string(FREE_CREDIT_DAILY) + " kredit gratis Anda.\n\n"
                "🔹 Untuk akses penuh tanpa batas, hubungi " + Config::ADMIN_CONTACT + "\n\n"
                "*💳 Pembayaran via bank*:\n"
                "- Bank: PM " + Config::ADMIN_CONTACT + " untuk nomor bank\n"
                "- Dana: PM " + Config::ADMIN_CONTACT + " untuk nomor Dana\n\n"
                "*💳 Pembayaran via crypto*:\n"
                "- BEP20: 0x68e1dcfd7a9a14e67f3a7d13db3719344b560b5d\n"
                "- TRC20: TKkpGHEYwhig1EK2Cnq2KUnJq8sFNjrZfx\n\n"
                "Setelah pembayaran, kirim bukti transfer ke " + Config::ADMIN_CONTACT;
                
            TgBot::Bot bot(Config::TOKEN);
            bot.getApi().sendMessage(userId, paymentMessage, false, 0, nullptr, "Markdown");
            return;
        }
    }
    
    TgBot::Bot bot(Config::TOKEN);
    bot.getApi().sendMessage(userId, "⚠️ Tidak dapat memverifikasi status pengguna.", false, 0, nullptr, "Markdown");
} 
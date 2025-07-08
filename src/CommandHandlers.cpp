#include "../include/CommandHandlers.h"
#include "../include/UserManager.h"
#include "../include/DatabaseManager.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include <tgbot/tgbot.h>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <fstream>
#include <filesystem>
#include <thread>
#include <future>
#include <random>
#include <chrono>
std::mutex CommandHandlers::busyLock;
std::unordered_set<std::string> CommandHandlers::closedCommands;
std::map<std::string, std::pair<int64_t, std::chrono::system_clock::time_point>> CommandHandlers::activeRequests;

void CommandHandlers::registerHandlers(TgBot::Bot& bot) {
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        start(message, bot);
    });
    
    bot.getEvents().onCommand("menu", [&bot](TgBot::Message::Ptr message) {
        menu(message, bot);
    });
    
    bot.getEvents().onCommand("mytokens", [&bot](TgBot::Message::Ptr message) {
        myTokens(message, bot);
    });
    
    bot.getEvents().onCommand("close", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(7); // Remove "/close "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        closeCommand(message, bot, argList);
    });
    
    bot.getEvents().onCommand("closed", [&bot](TgBot::Message::Ptr message) {
        closedCommand(message, bot);
    });
    
    bot.getEvents().onCommand("add_admin", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(11); // Remove "/add_admin "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        addAdmin(message, bot, argList);
    });
    
    bot.getEvents().onCommand("ban_admin", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(11); // Remove "/ban_admin "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        banAdmin(message, bot, argList);
    });
    
    bot.getEvents().onCommand("add", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(5); // Remove "/add "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        addUser(message, bot, argList);
    });
    
    bot.getEvents().onCommand("extend", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(8); // Remove "/extend "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        extendUser(message, bot, argList);
    });
    
    bot.getEvents().onCommand("dell", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(6); // Remove "/dell "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        dellUser(message, bot, argList);
    });
    
    bot.getEvents().onCommand("listuser", [&bot](TgBot::Message::Ptr message) {
        listUsers(message, bot);
    });
    
    bot.getEvents().onCommand("usertotal", [&bot](TgBot::Message::Ptr message) {
        userTotal(message, bot);
    });
    
    bot.getEvents().onCommand("dbtotal", [&bot](TgBot::Message::Ptr message) {
        dbTotal(message, bot);
    });
    
    bot.getEvents().onCommand("search", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(8); // Remove "/search "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        searchDomain(message, bot, argList);
    });
    
    bot.getEvents().onCommand("weblist", [&bot](TgBot::Message::Ptr message) {
        std::string args = message->text;
        args = args.substr(9); // Remove "/weblist "
        std::vector<std::string> argList;
        std::istringstream iss(args);
        for (std::string s; iss >> s;) {
            argList.push_back(s);
        }
        weblist(message, bot, argList);
    });
    
    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
        callbackQueryHandler(query, bot);
    });
}

void CommandHandlers::setupRepeatingJobs(TgBot::Bot& bot) {
    // Jalanin job berulang di thread terpisah biar ga boros CPU
    std::thread([&bot]() {
        while (true) {
            try {
                checkExpiryUsers(bot);
                std::this_thread::sleep_for(Config::EXPIRY_CHECK_INTERVAL);
            } catch (std::exception& e) {
                Logger::error("Error in checkExpiryUsers job: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
    }).detach();
    
    std::thread([&bot]() {
        while (true) {
            try {
                cleanupActiveRequests(bot);
                std::this_thread::sleep_for(Config::REQUEST_CLEANUP_INTERVAL);
            } catch (std::exception& e) {
                Logger::error("Error in cleanupActiveRequests job: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
    }).detach();
    
    std::thread([&bot]() {
        while (true) {
            try {
                keepAlive(bot);
                std::this_thread::sleep_for(Config::KEEP_ALIVE_INTERVAL);
            } catch (std::exception& e) {
                Logger::error("Error in keepAlive job: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
    }).detach();
}

// Command handlers
void CommandHandlers::start(const TgBot::Message::Ptr& message, TgBot::Bot& bot) {
    std::lock_guard<std::mutex> lock(busyLock);
    
    std::string welcomeMsg = 
        "👋 *Selamat datang di Infercloud ID Bot* 🎉\n\n"
        "🔹 *Pengguna terdaftar*: Nikmati akses penuh tanpa batas\n"
        "🔹 *Pengguna tidak terdaftar*: " + std::to_string(FREE_CREDIT_DAILY) + " kredit gratis (sekali pakai)\n\n"
        "Gunakan /menu untuk melihat opsi atau /mytokens untuk cek kredit Anda.";
    
    bot.getApi().sendMessage(message->chat->id, welcomeMsg, false, 0, nullptr, "Markdown");
}

void CommandHandlers::menu(const TgBot::Message::Ptr& message, TgBot::Bot& bot) {
    int64_t userId = message->from->id;
    
    if (!UserManager::isUserAllowed(userId, "menu")) {
        UserManager::handleNoTokens(*message->chat, "menu");
        return;
    }
    
    std::lock_guard<std::mutex> lock(busyLock);
    
    auto result = UserManager::getOrCreateUser(message->chat);
    auto user = result.first;
    auto unregisteredUser = result.second;
    
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
    
    std::vector<TgBot::InlineKeyboardButton::Ptr> row1;
    auto button1 = std::make_shared<TgBot::InlineKeyboardButton>();
    button1->text = "🔍 Cari Keyword";
    button1->callbackData = "search_domain";
    row1.push_back(button1);
    keyboard->inlineKeyboard.push_back(row1);
    
    std::vector<TgBot::InlineKeyboardButton::Ptr> row2;
    auto button2 = std::make_shared<TgBot::InlineKeyboardButton>();
    button2->text = "🌐 Daftar Web";
    button2->callbackData = "weblist";
    row2.push_back(button2);
    keyboard->inlineKeyboard.push_back(row2);
    
    std::vector<TgBot::InlineKeyboardButton::Ptr> row3;
    auto button3 = std::make_shared<TgBot::InlineKeyboardButton>();
    button3->text = "👥 Jumlah Pengguna";
    button3->callbackData = "usertotal";
    row3.push_back(button3);
    keyboard->inlineKeyboard.push_back(row3);
    
    std::vector<TgBot::InlineKeyboardButton::Ptr> row4;
    auto button4 = std::make_shared<TgBot::InlineKeyboardButton>();
    button4->text = "🗃️ Jumlah Database";
    button4->callbackData = "dbtotal";
    row4.push_back(button4);
    keyboard->inlineKeyboard.push_back(row4);
    
    // Add admin buttons if user is admin
    if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end() || 
        Config::FIRST_ADMIN_IDS.find(userId) != Config::FIRST_ADMIN_IDS.end() || 
        Config::SECOND_ADMIN_IDS.find(userId) != Config::SECOND_ADMIN_IDS.end()) {
        
        std::vector<TgBot::InlineKeyboardButton::Ptr> row5;
        auto button5 = std::make_shared<TgBot::InlineKeyboardButton>();
        button5->text = "➕ Tambah Pengguna";
        button5->callbackData = "add_user";
        row5.push_back(button5);
        keyboard->inlineKeyboard.push_back(row5);
        
        std::vector<TgBot::InlineKeyboardButton::Ptr> row6;
        auto button6 = std::make_shared<TgBot::InlineKeyboardButton>();
        button6->text = "➖ Hapus Pengguna";
        button6->callbackData = "dell_user";
        row6.push_back(button6);
        keyboard->inlineKeyboard.push_back(row6);
        
        std::vector<TgBot::InlineKeyboardButton::Ptr> row7;
        auto button7 = std::make_shared<TgBot::InlineKeyboardButton>();
        button7->text = "📋 Daftar Pengguna";
        button7->callbackData = "listuser";
        row7.push_back(button7);
        keyboard->inlineKeyboard.push_back(row7);
        
        // Add master admin buttons
        if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end()) {
            std::vector<TgBot::InlineKeyboardButton::Ptr> row8;
            auto button8 = std::make_shared<TgBot::InlineKeyboardButton>();
            button8->text = "👑 Tambah Admin";
            button8->callbackData = "add_admin";
            row8.push_back(button8);
            keyboard->inlineKeyboard.push_back(row8);
            
            std::vector<TgBot::InlineKeyboardButton::Ptr> row9;
            auto button9 = std::make_shared<TgBot::InlineKeyboardButton>();
            button9->text = "🚫 Ban Admin";
            button9->callbackData = "ban_admin";
            row9.push_back(button9);
            keyboard->inlineKeyboard.push_back(row9);
        }
    }
    
    // Add token check button for non-admin unregistered users
    if (!user.has_value() && 
        Config::MASTER_ADMIN_IDS.find(userId) == Config::MASTER_ADMIN_IDS.end() && 
        Config::FIRST_ADMIN_IDS.find(userId) == Config::FIRST_ADMIN_IDS.end() && 
        Config::SECOND_ADMIN_IDS.find(userId) == Config::SECOND_ADMIN_IDS.end()) {
        
        std::vector<TgBot::InlineKeyboardButton::Ptr> row10;
        auto button10 = std::make_shared<TgBot::InlineKeyboardButton>();
        button10->text = "🪙 Cek Kredit";
        button10->callbackData = "mytokens";
        row10.push_back(button10);
        keyboard->inlineKeyboard.push_back(row10);
    }
    
    // Determine user status
    std::string statusMsg;
    if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end()) {
        statusMsg = "👑 *Status*: Master Admin";
    } else if (Config::FIRST_ADMIN_IDS.find(userId) != Config::FIRST_ADMIN_IDS.end()) {
        statusMsg = "👑 *Status*: Admin Tingkat 1";
    } else if (Config::SECOND_ADMIN_IDS.find(userId) != Config::SECOND_ADMIN_IDS.end()) {
        statusMsg = "👑 *Status*: Admin Tingkat 2";
    } else if (user.has_value() && user->isActive()) {
        statusMsg = "🌟 *Status*: Pengguna Premium";
    } else {
        int tokens = unregisteredUser.has_value() ? unregisteredUser->getTokens() : FREE_CREDIT_DAILY;
        statusMsg = "🪙 *Kredit Gratis*: " + std::to_string(tokens) + "/" + std::to_string(FREE_CREDIT_DAILY);
    }
    
    std::string menuMsg = 
        "📋 *Menu Infercloud ID Bot* 📋\n\n" + statusMsg + "\n\n"
        "📌 *Pilih opsi di bawah atau gunakan perintah:*\n\n"
        "🔍 /search <keyword> - Cari keyword di database\n"
        "🌐 /weblist <tld> - Dapatkan daftar website berdasarkan TLD\n"
        "👥 /usertotal - Lihat jumlah pengguna aktif\n"
        "🗃️ /dbtotal - Lihat jumlah database\n"
        "🪙 /mytokens - Cek sisa kredit gratis\n";
    
    // Add admin commands
    if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end() || 
        Config::FIRST_ADMIN_IDS.find(userId) != Config::FIRST_ADMIN_IDS.end() || 
        Config::SECOND_ADMIN_IDS.find(userId) != Config::SECOND_ADMIN_IDS.end()) {
        
        menuMsg += 
            "\n*🔧 Perintah Admin:*\n"
            "➕ /add <id> <durasi> <unit> - Tambah pengguna baru\n"
            "➖ /dell <id> - Hapus pengguna\n"
            "📋 /listuser - Lihat daftar pengguna\n"
            "🔄 /extend <id> <durasi> <unit> - Perpanjang akses pengguna\n";
        
        // Add master admin commands
        if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end()) {
            menuMsg += 
                "\n*👑 Perintah Master Admin:*\n"
                "👑 /add_admin <id> <level> - Tambah admin (first/second)\n"
                "🚫 /ban_admin <id> - Ban admin\n"
                "🚫 /close <command> - Tutup perintah untuk non-admin\n"
                "📜 /closed - Lihat daftar perintah tertutup\n";
        }
    }
    
    bot.getApi().sendMessage(message->chat->id, menuMsg, false, 0, keyboard, "Markdown");
    
    UserManager::releaseUserRequest(userId);
}

void CommandHandlers::myTokens(const TgBot::Message::Ptr& message, TgBot::Bot& bot) {
    int64_t userId = message->from->id;
    
    // Admin check
    if (Config::MASTER_ADMIN_IDS.find(userId) != Config::MASTER_ADMIN_IDS.end() || 
        Config::FIRST_ADMIN_IDS.find(userId) != Config::FIRST_ADMIN_IDS.end() || 
        Config::SECOND_ADMIN_IDS.find(userId) != Config::SECOND_ADMIN_IDS.end()) {
        
        bot.getApi().sendMessage(message->chat->id, "👑 Anda adalah admin dengan akses penuh tanpa batas.", false, 0, nullptr, "Markdown");
        return;
    }
    
    std::lock_guard<std::mutex> lock(busyLock);
    
    auto result = UserManager::getOrCreateUser(message->chat);
    auto user = result.first;
    auto unregisteredUser = result.second;
    
    if (user.has_value() && user->isActive()) {
        bot.getApi().sendMessage(message->chat->id, "🌟 Anda adalah pengguna premium dengan akses penuh.", false, 0, nullptr, "Markdown");
    } else {
        int tokens = unregisteredUser.has_value() ? unregisteredUser->getTokens() : FREE_CREDIT_DAILY;
        std::string msg = 
            "🪙 *Sisa kredit gratis*: " + std::to_string(tokens) + "/" + std::to_string(FREE_CREDIT_DAILY) + "\n\n"
            "🔹 Setelah kredit habis, hubungi " + Config::ADMIN_CONTACT + " untuk upgrade ke akses premium.";
        
        bot.getApi().sendMessage(message->chat->id, msg, false, 0, nullptr, "Markdown");
    }
    
    UserManager::releaseUserRequest(userId);
}

void CommandHandlers::closeCommand(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args) {
    int64_t userId = message->from->id;
    
    if (Config::MASTER_ADMIN_IDS.find(userId) == Config::MASTER_ADMIN_IDS.end()) {
        bot.getApi().sendMessage(message->chat->id, "🚫 Hanya Master Admin yang dapat menggunakan perintah /close.", false, 0, nullptr, "Markdown");
        return;
    }
    
    std::lock_guard<std::mutex> lock(busyLock);
    
    if (args.size() != 1) {
        bot.getApi().sendMessage(message->chat->id, "📖 Penggunaan: /close <command> (contoh: /close weblist)", false, 0, nullptr, "Markdown");
        return;
    }
    
    std::string command = args[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    std::unordered_set<std::string> validCommands = {"weblist", "search", "usertotal", "dbtotal"};
    
    if (validCommands.find(command) == validCommands.end()) {
        bot.getApi().sendMessage(message->chat->id, 
            "❌ Perintah tidak valid: " + command + ". Pilih dari: weblist, search, usertotal, dbtotal",
            false, 0, nullptr, "Markdown");
        return;
    }
    
    if (closedCommands.find(command) != closedCommands.end()) {
        closedCommands.erase(command);
        bot.getApi().sendMessage(message->chat->id, "✅ Perintah /" + command + " telah dibuka kembali untuk semua pengguna.", false, 0, nullptr, "Markdown");
    } else {
        closedCommands.insert(command);
        bot.getApi().sendMessage(message->chat->id, "🔒 Perintah /" + command + " telah ditutup untuk pengguna non-admin.", false, 0, nullptr, "Markdown");
    }
}

void CommandHandlers::closedCommand(const TgBot::Message::Ptr& message, TgBot::Bot& bot) {
    int64_t userId = message->from->id;
    
    if (Config::MASTER_ADMIN_IDS.find(userId) == Config::MASTER_ADMIN_IDS.end()) {
        bot.getApi().sendMessage(message->chat->id, "🚫 Hanya Master Admin yang dapat menggunakan perintah /closed.", false, 0, nullptr, "Markdown");
        return;
    }
    
    std::lock_guard<std::mutex> lock(busyLock);
    
    if (!closedCommands.empty()) {
        std::string closedList;
        for (const auto& cmd : closedCommands) {
            closedList += "- /" + cmd + "\n";
        }
        bot.getApi().sendMessage(message->chat->id, "🔒 *Perintah yang Ditutup*:\n\n" + closedList, false, 0, nullptr, "Markdown");
    } else {
        bot.getApi().sendMessage(message->chat->id, "✅ Tidak ada perintah yang ditutup saat ini.", false, 0, nullptr, "Markdown");
    }
} 
#include "../include/Config.h"
#include "../include/Logger.h"
#include "../include/DatabaseManager.h"
#include "../include/CommandHandlers.h"
#include <tgbot/tgbot.h>
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <thread>
#include <exception>
TgBot::Bot* botPtr = nullptr;
bool shouldStop = false;

void signalHandler(int signum) {
    if (botPtr) {
        Logger::info("Stopping bot due to signal " + std::to_string(signum));
        shouldStop = true;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(signum);
}

int main() {
    try {
        Logger::info("Starting InfernoCloud ID Bot...");
        
        // Daftarin handler signal buat shutdown yang bersih
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        // Inisialisasi bot pake token
        TgBot::Bot bot(Config::TOKEN);
        botPtr = &bot;
        
        Logger::info("Bot initialized with username: " + bot.getApi().getMe()->username);
        
        // Inisialisasi database
        DatabaseManager::getInstance();
        Logger::info("Database manager initialized");
        
        // Daftarin handler command
        CommandHandlers::registerHandlers(bot);
        Logger::info("Command handlers registered");
        
        // Setup job yang bakal jalan terus
        CommandHandlers::setupRepeatingJobs(bot);
        Logger::info("Recurring jobs started");
        
        // Loop long polling
        TgBot::TgLongPoll longPoll(bot);
        Logger::info("Bot started. Press Ctrl+C to stop");
        
        while (!shouldStop) {
            try {
                longPoll.start();
            } catch (std::exception& e) {
                Logger::error("Error in long polling: " + std::string(e.what()));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        
        return EXIT_SUCCESS;
    } catch (std::exception& e) {
        Logger::critical("Fatal error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
} 
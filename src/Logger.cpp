#include "../include/Logger.h"
std::mutex Logger::logMutex;
std::ofstream Logger::logFile;
bool Logger::initialized = false; 
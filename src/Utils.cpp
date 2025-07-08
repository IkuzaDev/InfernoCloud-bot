#include "../include/UserManager.h"
#include "../include/Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <regex>

std::string Utils::getCurrentIsoDateTime() {
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&nowTime), "%Y-%m-%dT%H:%M:%S") << "Z";
    return ss.str();
}

std::chrono::system_clock::time_point Utils::parseIsoDateTime(const std::string& iso) {
    std::tm tm = {};
    std::istringstream ss(iso);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("Invalid date format. Expected format: YYYY-MM-DDThh:mm:ss");
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string Utils::formatDateTime(const std::chrono::system_clock::time_point& dt, const std::string& format) {
    auto time = std::chrono::system_clock::to_time_t(dt);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), format.c_str());
    return ss.str();
}

std::chrono::system_clock::time_point Utils::calculateExpiry(int duration, const std::string& unit) {
    auto now = std::chrono::system_clock::now();
    if (unit == "day") {
        return now + std::chrono::hours(duration * 24);
    }
    else if (unit == "month") {
        return now + std::chrono::hours(duration * 24 * 30);
    }
    else if (unit == "minute") {
        return now + std::chrono::minutes(duration);
    }
    else {
        throw std::invalid_argument("Unit must be 'day', 'month', or 'minute'");
    }
}

std::string Utils::sanitizeFilename(const std::string& input) {
    std::regex invalidChars("[^\\w\\-_\\. ]");
    return std::regex_replace(input, invalidChars, "_");
}

bool Utils::filterUnwantedLine(const std::string& line) {
    std::vector<std::string> unwantedKeywords = {"t.me", "telegram", "@", "subscribe"};
    std::string lineLower = line;
    std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), ::tolower);
    
    for (const auto& keyword : unwantedKeywords) {
        if (lineLower.find(keyword) != std::string::npos) {
            return false;
        }
    }
    
    return true;
} 
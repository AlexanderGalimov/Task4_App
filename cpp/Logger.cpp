#include "../header/Logger.h"
#include <fstream>
#include <ctime>
#include <stdexcept>
#include <utility>

Logger::Logger(std::string filename) : logFilename(std::move(filename)) {}

void Logger::logMessage(const std::string &message, const std::string &importance) {
    std::ofstream logFile(logFilename, std::ios_base::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Unable to open log file");
    }

    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    logFile << "[" << buf << "] [" << importance << "] " << message << std::endl;
}
#pragma once
#include <iostream>
#include <mutex>


namespace common_utils {


typedef enum eLogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
} LogLevel;


class Logger {
private:
    // Data
    std::mutex m_lock;
    LogLevel log_level = LogLevel::INFO;
    std::ostream* os = &std::cout;
    inline static Logger* _instance = nullptr;
private:
    // Methods
    Logger() = default;
    Logger(const Logger&) = default;
    Logger& operator=(const Logger&) = default;

    template <class... Args>
    void logMessage(const LogLevel& level, const Args&... args) {
        if (log_level <= level) {
            std::unique_lock<std::mutex> lock(m_lock);
            *os << '[' << stringifyLevel(level) << "]: ";
            ((*os << args), ...);
            *os << '\n';
        }
    };

    std::string stringifyLevel(const LogLevel& level) {
        // Workaround to translate from enum type into a string
        std::string str;
        switch (level) {
            case DEBUG: str = "DEBUG"; break;
            case INFO: str = "INFO"; break;
            case WARNING: str = "WARN"; break;
            case ERROR: str = "ERROR"; break;
            default: str = ""; break;
        }
        return str;
    };

public:
    // Methods
    static Logger& getInstance() {
        if (_instance == nullptr)
            _instance = new Logger();
        return *_instance;
    };

    template <class... Args>
    void debug(const Args&... args) {
        this->logMessage(LogLevel::DEBUG, args...);
    };

    template <class... Args>
    void info(const Args&... args) {
        this->logMessage(LogLevel::INFO, args...);
    };

    template <class... Args>
    void warning(const Args&... args) {
        this->logMessage(LogLevel::WARNING, args...);
    };
    
    template <class... Args>
    void error(const Args&... args) {
        this->logMessage(LogLevel::ERROR, args...);
    };

    void setLogLevel(LogLevel new_log_level) {
        std::unique_lock<std::mutex> lock(m_lock);
        log_level = new_log_level;
    };

    void setStream(std::ostream& stream) {
        std::unique_lock<std::mutex> lock(m_lock);
        os = &stream;
    };
};


} // end of namespace common_utils;
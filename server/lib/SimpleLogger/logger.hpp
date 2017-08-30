#pragma once

#include <cstdarg>
#include <iostream>
#include <functional>
#include <string>


namespace Log {
    enum class LogLevel {
        DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3
    };


    class SimpleLogger {
    private:
        FILE *_out;
        LogLevel _logLevel;
        bool _printTimestamp = false;
        std::function<std::string(void)> _time;

        SimpleLogger() :
            _out(stdout),
            _logLevel(LogLevel::INFO),
            _time([]() { return ""; }) {
        }

    public:
        ~SimpleLogger() {
            fclose(_out);
        }


        static SimpleLogger& getInstance() {
            static SimpleLogger instance;
            return instance;
        }


        SimpleLogger(const SimpleLogger&) = delete;


        void operator=(const SimpleLogger&) = delete;


        void setLogLevel(const LogLevel level) {
            _logLevel = level;
        }


        LogLevel getLogLevel() const {
            return _logLevel;
        }


        static std::string logLevelAsString(const LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARNING:
                    return "WARNING";
                case LogLevel::ERROR:
                    return "ERROR";
            }
            return "UNKNOWN";
        }


        void addTimestamp(const std::function<std::string(void)>& f) {
            _printTimestamp = true;
            _time = f;
        }


        void hideTimestamp() {
            _printTimestamp = false;
        }


        void log(const LogLevel logLevel, const char *msg, va_list args) {
            if (logLevel >= _logLevel) {
                auto logLevel_str = logLevelAsString(logLevel);
                if (_printTimestamp) {
                    auto formatedTime = _time();
                    fprintf(_out, "[%s] %s", logLevel_str.c_str(),
                                             formatedTime.c_str());
                } else {
                    fprintf(_out, "[%s]", logLevel_str.c_str());
                }

                vfprintf(_out, msg, args);
                fprintf(_out, "\n");
            }
        }
    };


    void debug(const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        SimpleLogger::getInstance().log(LogLevel::DEBUG, msg, args);
        va_end(args);
    }


    void info(const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        SimpleLogger::getInstance().log(LogLevel::INFO, msg, args);
        va_end(args);
    }


    void warning(const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        SimpleLogger::getInstance().log(LogLevel::WARNING, msg, args);
        va_end(args);
    }

    void error(const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        SimpleLogger::getInstance().log(LogLevel::ERROR, msg, args);
        va_end(args);
    }
}

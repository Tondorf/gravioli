#include <cassert>
#include <iostream>
#include <cstdarg>

#include "logger.h"

namespace Log {
    SimpleLogger::SimpleLogger() : _out(stdout), _logLevel(LogLevel::INFO) {
    }

    SimpleLogger::~SimpleLogger() {
        fclose(_out);
    }

    SimpleLogger &SimpleLogger::getInstance() {
        static SimpleLogger instance;
        return instance;
    }

    void SimpleLogger::setLogLevel(const LogLevel logLevel) {
        _logLevel = logLevel;
    }

    LogLevel SimpleLogger::getLogLevel() const {
        return _logLevel;
    }

    void SimpleLogger::log(const LogLevel logLevel, const char *msg, va_list args) {
        if (logLevel >= _logLevel) {
            switch (logLevel) {
                case LogLevel::DEBUG:
                    fprintf(_out, "[DEBUG] ");
                    break;
                case LogLevel::INFO:
                    fprintf(_out, "[INFO] ");
                    break;
                case LogLevel::ERROR:
                    fprintf(_out, "[ERROR] ");
                    break;
                default:
                    assert(false);
            }

            vfprintf(_out, msg, args);
            fprintf(_out, "\n");
        }
    }

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

    void error(const char *msg, ...) {
        va_list args;
        va_start(args, msg);
        SimpleLogger::getInstance().log(LogLevel::ERROR, msg, args);
        va_end(args);
    }
}

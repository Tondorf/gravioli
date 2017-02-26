#pragma once

#include <string>

namespace Log {
    enum class LogLevel {
        DEBUG = 0, INFO = 1, ERROR = 2
    };

    class SimpleLogger {
    private:
        FILE *_out;
        LogLevel _logLevel;

        SimpleLogger();

    public:
        ~SimpleLogger();

        static SimpleLogger &getInstance();

        SimpleLogger(SimpleLogger const &) = delete;

        void operator=(SimpleLogger const &) = delete;

        void setLogLevel(const LogLevel);

        LogLevel getLogLevel() const;

        void log(const LogLevel logLevel, const char *msg, va_list);
    };

    void debug(const char *, ...);

    void info(const char *, ...);

    void error(const char *, ...);
};
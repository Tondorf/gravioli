#pragma once

#include <chrono>
#include <iomanip>
#include <string>

#include "config.hpp"


class Greenwich {
private:
    const std::string _format;
    const std::string _locale;

    Greenwich(std::string format = "", std::string locale = ""):
        _format(format.empty() ? LOGDATE_FORMAT : format),
        _locale(locale.empty() ? LOGDATE_LOCALE : locale) {

        std::locale::global(std::locale(_locale.c_str()));

        /*
         * Do not use Logging here, since this constructor
         * is called before Logging is configured properly.
         */
    }

    static Greenwich& getNonConstInstance() {
        static Greenwich instance;
        return instance;
    }

public:
    ~Greenwich() = default;

    static const Greenwich& getInstance() {
        return getNonConstInstance();
    }

    Greenwich(const Greenwich&) = delete;

    void operator=(const Greenwich&) = delete;

    std::string getFormatedTime() const {
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), _format.c_str());

        return ss.str();
    }
};

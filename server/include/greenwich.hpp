#pragma once

#include <string>


class Greenwich {
private:
    const std::string _format;
    const std::string _locale;

    Greenwich(std::string format = "", std::string locale = "");

    static Greenwich& getNonConstInstance();

public:
    virtual ~Greenwich() = default;

    static const Greenwich& getInstance();

    Greenwich(const Greenwich&) = delete;

    void operator=(const Greenwich&) = delete;

    std::string getFormatedTime() const;
};

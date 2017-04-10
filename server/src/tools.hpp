#include <ctime>
#include <locale>
#include <string>

#include "config.h"


std::string getFormatedTime(std::string format = "", std::string locale = "") {
    if (format.empty()) {
        format = LOGDATE_FORMAT;
    }

    if (locale.empty()) {
        locale = LOGDATE_LOCALE;
    }

    std::locale::global(std::locale(locale.c_str()));

    std::time_t t = std::time(NULL);
    char tasstr[100];
    std::strftime(tasstr, sizeof(tasstr), format.c_str(), std::localtime(&t));

    return tasstr;
}

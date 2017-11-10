#include "greenwich.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>
#include "config.hpp"


Greenwich::Greenwich(std::string format, std::string locale):
    _format(format.empty() ? LOGDATE_FORMAT : format),
    _locale(locale.empty() ? LOGDATE_LOCALE : locale) {

    std::locale::global(std::locale(_locale.c_str()));

    /*
     * Do not use Logging here, since this constructor
     * is called before Logging is configured properly.
     */
}


Greenwich& Greenwich::getNonConstInstance() {
    static Greenwich instance;
    return instance;
}


const Greenwich& Greenwich::getInstance() {
    return getNonConstInstance();
}


std::string Greenwich::getFormatedTime() const {
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    );

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), _format.c_str());

    return ss.str();
}

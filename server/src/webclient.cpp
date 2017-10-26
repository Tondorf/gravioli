#include "webclient.hpp"


namespace utils  {
    WebClient::WebClient() {
    }


    std::string WebClient::get(const std::string& url, int& statusCode) {
        if (url == "players") {
            statusCode = 200;
            return "{ \"IDs\": [0, 1] }";
        } else if (url == "player/0") {
            statusCode = 200;
            return "{ \"key\": [ 0,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }";
        } else if (url == "player/1") {
            statusCode = 200;
            return "{ \"key\": [ 1,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }";
        } else {
            statusCode = 404;
            return "";
        }
    }
}

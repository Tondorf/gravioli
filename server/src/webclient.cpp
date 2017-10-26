#include "webclient.hpp"


namespace utils  {
    WebClient::WebClient() {
    }


    std::string WebClient::get(const std::string& url, int& statusCode) {
        if (url == "players") {
            statusCode = 200;
            return "{ IDs: [0, 1] }";
        } else if (url == "player/0") {
            statusCode = 200;
            return "{ key: \"000102030405060708090a0b0c0d0e0f\" }";
        } else if (url == "player/1") {
            statusCode = 200;
            return "{ key: \"010102030405060708090a0b0c0d0e0f\" }";
        } else {
            statusCode = 404;
            return "";
        }
    }
}

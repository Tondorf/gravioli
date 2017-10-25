#include "webclient.hpp"


namespace utils  {
    WebClient::WebClient() {
    }


    std::string get(const std::string& /*url*/, int& statusCode) {
        statusCode = 200;
        return "{ key: 42 }";
    }
}

#include "gravioliWebClient.hpp"


namespace gravioli  {
    GravioliWebClient::GravioliWebClient() {
    }


    GravioliWebClient::Response GravioliWebClient::get(const std::string& url) {
        if (url == "players") {
            return {"{ \"IDs\": [0, 1] }", 200};
        } else if (url == "player/0") {
            return {"{ \"key\": [ 0,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }", 200};
        } else if (url == "player/1") {
            return {"{ \"key\": [ 1,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }", 200};
        } else {
            return {"", 404};
        }
    }
}

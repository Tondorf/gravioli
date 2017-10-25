#pragma once

#include <string>


namespace utils {
    class WebClient {
    public:
        WebClient();

        virtual ~WebClient() = default;

        std::string get(const std::string& url, int& statusCode);
    };
}

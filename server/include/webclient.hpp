#pragma once

#include <string>


namespace utils {
    class WebClient {
    public:
        WebClient();

        virtual ~WebClient() = default;

        struct Response {
            std::string content;
            int statusCode;
        };
        virtual Response get(const std::string& url);
    };
}

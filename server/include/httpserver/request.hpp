#pragma once

#include <string>
#include <vector>

#include "httpserver/header.hpp"


namespace httpserver {
    struct Request {
        std::string method;
        std::string uri;
        int http_version_major;
        int http_version_minor;
        std::vector<Header> headers;
    };
}

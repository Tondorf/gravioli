#pragma once

#include <string>


namespace httpserver {
    struct Header {
        std::string name;
        std::string value;
    };
}

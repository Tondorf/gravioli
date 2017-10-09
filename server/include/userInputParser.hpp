#pragma once

#include "config.hpp"


struct Config {
    port_t port;
};

bool parseUserInput(int argc, char const* argv[], Config&);

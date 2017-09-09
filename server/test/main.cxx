#include <gtest/gtest.h>

#include "SimpleLogger/logger.hpp"


int main(int argc, char **argv) {
    Log::SimpleLogger::getInstance().setLogLevel(Log::LogLevel::ERROR);
    Log::SimpleLogger::getInstance().addTimestamp([]() {
        return "";
    });

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

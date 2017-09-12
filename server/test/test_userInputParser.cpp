#include <gtest/gtest.h>

#include "common/userInputParser.hpp"


class UserInputParser : public ::testing::Test {
public:
    std::vector<const char*> argv;
    
    UserInputParser() {
    }

    void SetUp() override {
        // there is always at least one argument,
        // i.e. the name of the exectuable
        argv.push_back("./server");
    }

    void TearDown() {
        argv.clear();
    }

    virtual ~UserInputParser() = default;
};


inline bool parseUserInput(std::vector<const char*> &argv, Config &config) {
    return parseUserInput(argv.size(), &argv[0], config);
}

inline port_t getNonDefaultPort() {
    return DEFAULT_PORT > 10000 ? 10000 : 20000;
}

TEST(DefaultValues, setsValidUserPort) { 
    ASSERT_GE(DEFAULT_PORT, 1024);
    ASSERT_LE(DEFAULT_PORT, 49151);
}

TEST_F(UserInputParser, setsDefaultValuesOnEmpty) { 
    Config config;
    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(DEFAULT_PORT, config.port);
}

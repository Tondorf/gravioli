#include <gtest/gtest.h>

#include "../src/userInputParser.hpp"


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


bool parseUserInput(std::vector<const char*> &argv, Config &config) {
    return parseUserInput(argv.size(), &argv[0], config);
}

port_t getNonDefaultPort() {
    return DEFAULT_PORT > 10000 ? 10000 : 20000;
}

std::size_t getNonDefaultThreads() {
    return DEFAULT_THREADS + 1;
}


TEST(DefaultValues, setsValidUserPort) { 
    ASSERT_GE(DEFAULT_PORT, 1024);
    ASSERT_LE(DEFAULT_PORT, 49151);
}

TEST(DefaultValues, setsThreadsToOne) {
    ASSERT_LE(DEFAULT_THREADS, 1);
}

TEST_F(UserInputParser, setsDefaultValuesOnEmpty) { 
    Config config;
    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(DEFAULT_PORT, config.port);
    ASSERT_EQ(DEFAULT_THREADS, config.threads);
}

TEST_F(UserInputParser, setsPortFromUserInput) { 
    Config config;

    argv.push_back("--port");

    auto nonDefaultPort = getNonDefaultPort();
    auto nonDefaultPort_asString = std::to_string(nonDefaultPort);
    argv.push_back(nonDefaultPort_asString.c_str());

    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(nonDefaultPort, config.port);
    ASSERT_EQ(DEFAULT_THREADS, config.threads);
}

TEST_F(UserInputParser, setsThreadFromUserInput) { 
    Config config;

    argv.push_back("--threads");

    auto nonDefaultThreads = getNonDefaultThreads();
    auto nonDefaultThreads_asString = std::to_string(nonDefaultThreads);
    argv.push_back(nonDefaultThreads_asString.c_str());

    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(DEFAULT_PORT, config.port);
    ASSERT_EQ(nonDefaultThreads, config.threads);
}

TEST_F(UserInputParser, setsPortAndThreadsFromUserInput) { 
    Config config;

    argv.push_back("--port");

    auto nonDefaultPort = getNonDefaultPort();
    auto nonDefaultPort_asString = std::to_string(nonDefaultPort);
    argv.push_back(nonDefaultPort_asString.c_str());

    argv.push_back("--threads");

    auto nonDefaultThreads = getNonDefaultThreads();
    auto nonDefaultThreads_asString = std::to_string(nonDefaultThreads);
    argv.push_back(nonDefaultThreads_asString.c_str());

    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(nonDefaultPort, config.port);
    ASSERT_EQ(nonDefaultThreads, config.threads);
}

TEST_F(UserInputParser, setsPortAndThreadsFromUserInputInReversedOrder) { 
    Config config;

    argv.push_back("--threads");

    auto nonDefaultThreads = getNonDefaultThreads();
    auto nonDefaultThreads_asString = std::to_string(nonDefaultThreads);
    argv.push_back(nonDefaultThreads_asString.c_str());

    argv.push_back("--port");

    auto nonDefaultPort = getNonDefaultPort();
    auto nonDefaultPort_asString = std::to_string(nonDefaultPort);
    argv.push_back(nonDefaultPort_asString.c_str());

    auto returnValue = parseUserInput(argv, config);

    ASSERT_EQ(returnValue, true);
    ASSERT_EQ(nonDefaultPort, config.port);
    ASSERT_EQ(nonDefaultThreads, config.threads);
}

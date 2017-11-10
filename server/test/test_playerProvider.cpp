#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "crypto.hpp"
#include "player.hpp"
#include "playerProvider.hpp"
#include "webclient.hpp"


class TestWebClient: public utils::WebClient {
public:
    TestWebClient() {
    }

    virtual ~TestWebClient() = default;

    virtual std::string get(const std::string& url, int& statusCode) override {
        if (url == "players") {
            statusCode = 200;
            return "{ \"IDs\": [0, 1] }";
        } else if (url == "player/0") {
            statusCode = 200;
            return "{ \"key\": [ 0,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }";
        } else if (url == "player/1") {
            statusCode = 200;
            return "{ \"key\": [ 1,  1,  2,  3, "
                                "4,  5,  6,  7, "
                                "8,  9, 10, 11, "
                               "12, 13, 14, 15] }";
        } else {
            statusCode = 404;
            return "";
        }
    }
};


class TestPlayerProvider : public simulation::PlayerProvider {
public:
    TestPlayerProvider():
        simulation::PlayerProvider() {
        simulation::PlayerProvider::_webclient = TestWebClient();
    }

    virtual ~TestPlayerProvider() = default;

    std::vector<int> getPlayerIDs() {
        return simulation::PlayerProvider::getPlayerIDs();
    }

    std::shared_ptr<simulation::Player> getPlayerById(int id,
                                                      bool expect200OK = true) {
        return simulation::PlayerProvider::getPlayerById(id, expect200OK);
    }

    PlayerDataStatus getPlayerDataStatus() {
        return _playerDataStatus;
    }
};


class PlayerProvider : public ::testing::Test {
public:
    TestPlayerProvider playerProvider;

    PlayerProvider() {
    }

    ~PlayerProvider() = default;
};


TEST_F(PlayerProvider, readsPlayerIDsFromJSON) {
   auto ids = playerProvider.getPlayerIDs(); 
   ASSERT_EQ(ids.size(), 2);
   ASSERT_EQ(ids[0], 0);
   ASSERT_EQ(ids[1], 1);
}


inline ::testing::AssertionResult equal(const simulation::Player& a,
                                        const simulation::Player& b) {

    constexpr auto N = crypto::KEY_BLOCKSIZE;
    auto k1 = a.getKey();
    auto k2 = b.getKey();
    for (std::size_t i = 0; i < N; ++i) {
        auto k1i = static_cast<int>(k1[i]);
        auto k2i = static_cast<int>(k2[i]);
        if (k1i != k2i) {
            return ::testing::AssertionFailure() <<
                   "keys differ; " << k1i << " != " << k2i;
        }
    }

    if (a.getID() != b.getID()) {
        return ::testing::AssertionFailure() <<
               "IDs differ; " << a.getID() << " != " << b.getID();
    }

    return ::testing::AssertionSuccess();
}


TEST_F(PlayerProvider, returnNullptrForNonExistingPlayer) {
    auto player = playerProvider.getPlayerById(2, false);
    ASSERT_EQ(player, nullptr);
}


TEST_F(PlayerProvider, readsPlayerFromJSON) {
    auto player1 = playerProvider.getPlayerById(0);
    auto player2 = playerProvider.getPlayerById(1);

    ASSERT_FALSE(equal(*player1, *player2));

    ASSERT_TRUE(equal(*player1, simulation::Player(0, crypto::Key {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    })));

    ASSERT_TRUE(equal(*player2, simulation::Player(1, crypto::Key {
        0x01, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    })));
}

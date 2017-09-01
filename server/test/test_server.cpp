#include <gtest/gtest.h>

#include <thread>

#include "simpleMsgQueue.hpp"

#include "helper.hpp"


class Server : public ::testing::Test {
public:
    using msg_t = server::IMsgQueue::Messages::msg_t;

    std::shared_ptr<server::SimpleMsgQueue> msgQueue;
    server::Server server;
    std::thread serverThread;

    Server() : msgQueue(std::make_shared<server::SimpleMsgQueue>()),
               server(8888, msgQueue),
               serverThread([this]() { this->server.run(); }) {
    }

    virtual ~Server() {
        server.stop();
        serverThread.join();
    }
};


/*
 * TODO
 *
 * implement test client and whether 
 * -> client receive message
 * -> message is correctly encrypted
 *
 */
TEST_F(Server, cryptsMessages) { 
    constexpr crypto::Key TESTKEY {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };

    auto *msg = createMsg(TESTKEY, {
        0x04, 0x08, 0x15, 0x10, 0x17, 0x2a
    });

    /*
     * TODO
     *
     * remember to pass a valid deallocation function
     */
    std::vector<msg_t> msgs {
        std::make_pair(msg, nullptr)
    };

    /*
     * TODO
     *
     * not necessary if message was submitted successfully
     */
    delete msg;

    ASSERT_TRUE(false);
}

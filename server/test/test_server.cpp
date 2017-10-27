#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <future>
#include <thread>

#include "SimpleLogger/logger.hpp"

#include "simpleMsgQueue.hpp"

#include "helper.hpp"


using namespace std::chrono_literals;


constexpr int CLIENT_TIMEOUT = 1000; // timeout in ms


class TestServer: public server::Server {
private:
    std::size_t _counter;

public:
    TestServer(const TestServer&) = delete;

    TestServer& operator=(const TestServer&) = delete;

    TestServer(port_t port, std::shared_ptr<server::IMsgQueue> queue):
        Server(port, queue),
        _counter(0) {
    }

    virtual ~TestServer() = default;

    void sleep() override {
        std::this_thread::sleep_for(100ms);
    }

    bool process(server::IMsgQueue::Messages&& popped) override {
        _counter += 1;
        Log::debug("Server processes message #%d", _counter);
        return server::Server::process(std::move(popped));
    }
};

class Server: public ::testing::Test {
public:
    std::shared_ptr<server::SimpleMsgQueue> msgQueue;
    TestServer server;
    std::thread serverThread;
    
    void *clientContext; 
    void *clientSocket;

    Server() :
        msgQueue(std::make_shared<server::SimpleMsgQueue>()),
        server(8888, msgQueue),
        serverThread([this]() { this->server.run(); }),
        clientContext(zmq_ctx_new()),
        clientSocket(zmq_socket(clientContext, ZMQ_SUB)) {

        /*
         * Sleep for a short period of time to prevent slow starter syndrome
         */
        std::this_thread::sleep_for(100ms);
        zmq_connect(clientSocket, "tcp://localhost:8888");
        zmq_setsockopt(clientSocket, ZMQ_RCVTIMEO, &CLIENT_TIMEOUT,
                                                   sizeof(CLIENT_TIMEOUT));
    }

    virtual ~Server() {
        zmq_close(clientSocket);
        zmq_ctx_destroy(clientContext);

        server.stop();
        serverThread.join();
    }

    void subscribeClient(const std::vector<byte>& topic) {
        zmq_setsockopt(clientSocket, ZMQ_SUBSCRIBE, &topic[0], topic.size());
    }

    bool recvNextMsgAsClient(std::vector<byte>& bytes, bool& more) {
        zmq_msg_t part;

        zmq_msg_init(&part);

        {
            int rc = zmq_recvmsg(clientSocket, &part, 0);
            if (rc == -1) {
                Log::error("Error during receiving: %s", zmq_strerror(errno));
                zmq_msg_close(&part); 
                return false;
            }
        }

        const auto size = zmq_msg_size(&part);

        // std::vector::reserve does not work with std::memcpy
        bytes = std::vector<byte>(size);
        std::memcpy(bytes.data(), zmq_msg_data(&part), size);

        more = [socket = this->clientSocket]() {
            std::int64_t more;
            auto sizeOfMore = sizeof(more);
            zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &sizeOfMore);
            return (more != 0);
        }();

        zmq_msg_close(&part); 

        return true;
    }
};


inline ::testing::AssertionResult equal(const std::vector<byte>& a,
                                        const std::vector<byte>& b) {
    auto sizeOfA = a.size();
    auto sizeOfB = b.size();
    if (sizeOfA != sizeOfB) {
        return ::testing::AssertionFailure() <<
               "amount of stored data differs" <<
               "; " << sizeOfA << " != " << sizeOfB;
    }

    for (std::size_t i = 0; i < sizeOfA; ++i) {
        if (a[i] != b[i]) {
            return ::testing::AssertionFailure() << 
                   "stored data mismatch at postion " << i <<
                   "; " << static_cast<int>(a[i]) <<
                   " != " << static_cast<int>(b[i]);
        }
    }

    return ::testing::AssertionSuccess();
}


inline ::testing::AssertionResult noErrorDuringRecv(bool rc) {
    if (rc) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::AssertionFailure() << 
           "error during receiving. Maybe a timeout?";
}


TEST(ToLittleEndian, ConvertsIntegerCorrectly) {
    const int x = 256;
    auto bytes = server::Server::toLittleEndian(x);

    ASSERT_TRUE(equal(bytes, {
        0x00,
        0x00,
        0x01,
        0x00
    }));
}


constexpr crypto::Key TESTKEY {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
    0x0c, 0x0d, 0x0e, 0x0f
};


inline server::IMsgQueue::Messages createNewTestMessages(
    const std::vector<byte>& testData,
    const std::uint32_t topicID) {

    auto *msg = createMsg(TESTKEY, testData);
    ++currentlyAllocatedMsgInstances;

    return server::IMsgQueue::Messages(topicID, {
        std::make_pair(msg, &deleteMsg)
    });
}


TEST_F(Server, cryptsMessages) { 
    constexpr std::size_t NMSGS = 1;
    const std::vector<byte> testData {
        0x04, 0x08, 0x15, 0x10, 0x17, 0x2a
    };
    constexpr std::uint32_t topicID = 108;
    subscribeClient(server::Server::toLittleEndian(topicID));

    std::vector<server::IMsgQueue::Messages> vmsgs;

    ASSERT_EQ(currentlyAllocatedMsgInstances.load(), 0);
    for (std::size_t i = 0; i < NMSGS; ++i) {
        vmsgs.push_back(std::move(createNewTestMessages(testData, topicID)));
    }
    ASSERT_EQ(currentlyAllocatedMsgInstances.load(), NMSGS);

    using bytes = std::vector<byte>;
    std::vector<bytes> bytesRecvByClient;

    auto rc = std::async(std::launch::async, [&]() {
        bytes b;
        bool more;
        do {
            Log::debug("Client is waiting for a new message ...");
            bool rc = recvNextMsgAsClient(b, more);
            Log::debug("Client received a new message.");
            if (!rc || bytesRecvByClient.size() > 2) {
                return false;
            }
            bytesRecvByClient.push_back(b);
        } while (more);
        return true;
    });

    for (auto&& msgs : vmsgs) {
        Log::debug("Pushing new messages ...");
        msgQueue->push(std::move(msgs)); 
//        std::this_thread::sleep_for(100ms);
    }

    /*
     * rc.get() blocks until either
     * -> all messages are received by the client, or
     * -> an error occurs
     */
    ASSERT_TRUE(noErrorDuringRecv(rc.get()));
    ASSERT_EQ(bytesRecvByClient.size(), 3);

    struct RecvBytes {
        bytes topicID;
        bytes iv;
        bytes cipher;
    } recvBytes;

    recvBytes.topicID = std::move(bytesRecvByClient[0]);
    recvBytes.iv = std::move(bytesRecvByClient[1]);
    recvBytes.cipher = std::move(bytesRecvByClient[2]);
    
    // topic ID
    ASSERT_TRUE(equal(
        recvBytes.topicID,
        server::Server::toLittleEndian(topicID)
    ));
    Log::debug("Topic ID was submitted successfully");

    // IV
    crypto::IV iv;
    ASSERT_EQ(recvBytes.iv.size(), iv.size());
    Log::debug("IV was submitted successfully");
    std::copy_n(
        std::make_move_iterator(recvBytes.iv.begin()),
        iv.size(),
        iv.begin()
    );

    // CIPHER
    crypto::decrypt(&recvBytes.cipher[0], recvBytes.cipher.size(),
                    TESTKEY, iv);
    ASSERT_TRUE(equal(recvBytes.cipher, testData));
    Log::debug("Cipher was submitted successfully");

    for (std::size_t i= 0; currentlyAllocatedMsgInstances.load() != 0; ++i) {
        if (i > 10) {
            break;
        }
        std::this_thread::sleep_for(10ms);
    }
    ASSERT_EQ(currentlyAllocatedMsgInstances.load(), 0);
}

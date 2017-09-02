#include <gtest/gtest.h>

#include <cstring>
#include <thread>

#include "simpleMsgQueue.hpp"

#include "helper.hpp"


class Server : public ::testing::Test {
public:
    using msg_t = server::IMsgQueue::Messages::msg_t;

    std::shared_ptr<server::SimpleMsgQueue> msgQueue;
    server::Server server;
    std::thread serverThread;
    
    void *clientContext; 
    void *clientSocket;

    Server() :
        msgQueue(std::make_shared<server::SimpleMsgQueue>()),
        server(8888, msgQueue),
        serverThread([this]() { this->server.run(); }),
        clientContext(zmq_ctx_new()),
        clientSocket(zmq_socket(clientContext, ZMQ_SUB)) {

        zmq_connect(clientSocket, "tcp://localhost:8888");
    }

    virtual ~Server() {
        zmq_close(clientSocket);
        zmq_ctx_destroy(clientContext);

        server.stop();
        serverThread.join();
    }

    bool recvNextMsgAsClient(std::vector<byte>& bytes) {
        zmq_msg_t part;

        zmq_msg_init(&part);
        zmq_recvmsg(clientSocket, &part, 0);

        const auto size = zmq_msg_size(&part);
        bytes.reserve(size);
        std::memcpy(bytes.data(), zmq_msg_data(&part), size);

        std::int64_t more;
        auto sizeOfMore = sizeof(more);
        zmq_getsockopt(clientSocket, ZMQ_RCVMORE, &more, &sizeOfMore);

        zmq_msg_close(&part); 

        return (more != 0);
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

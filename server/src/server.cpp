#include <chrono>
#include <cstring>
#include <thread>

#include "server.hpp"

#include "crypto.hpp"

#include "SimpleLogger/logger.hpp"


namespace server {
    bool Server::sendMessage(zmq_msg_t *msg, 
                             const std::size_t size,
                             bool more) {
        int rc = zmq_msg_send(msg, _publisher, more ? ZMQ_SNDMORE : 0);
        if (static_cast<std::size_t>(rc) != size) {
            return false;
        }

        return true;
    }


    bool Server::sendBytes(byte *bytes,
                           const std::size_t size,
                           bool more) {
        zmq_msg_t msg;

        int rc = zmq_msg_init_size(&msg, size);
        if (rc != 0) {
            return false;
        }

        memcpy(zmq_msg_data(&msg), bytes, size);

        return sendMessage(&msg, size, more);
    }


    bool Server::sendBytes(byte *bytes,
                           CustomFreePtr_t customFree,
                           void *memOwner,
                           const std::size_t size,
                           bool more) {
        if (customFree == nullptr) {
            return sendBytes(bytes, size, more);
        }

        zmq_msg_t msg;
        
        int rc = zmq_msg_init_data(&msg, bytes, size, customFree, memOwner);
        if (rc != 0) {
            return false;
        }

        return sendMessage(&msg, size, more);
    }


    bool Server::cryptAndSendBytes(byte *bytes, 
                                   CustomFreePtr_t customFree,
                                   void *memOwner,
                                   const std::size_t size,
                                   const crypto::Key& key,
                                   bool more) {
        auto iv = crypto::encrypt(bytes, size, key);

        /*
         * zeromq allocates memory on the heap, when 'size' is above some
         * threshold. In this case it is faster to have the data dynamically
         * allocated, too and save the memcpy step.
         * Typically, IV is below threshold, therefore the technique using
         * memcpy is faster / non-critically.
         */
        return sendBytes(&iv[0], crypto::IV_BLOCKSIZE, true) &&
               sendBytes(bytes, customFree, memOwner, size, more);
    }


    Server::Server(port_t port, std::shared_ptr<IMsgQueue> msgQueue):
        _connected(false),
        _stopped(false),
        _port(port),
        _context(zmq_ctx_new()),
        _publisher(zmq_socket(_context, ZMQ_PUB)),
        _msgQueue(msgQueue) {

        Log::info("Server listens on port %d", port);

        std::string endpoint = std::string("tcp://*:")
                             + std::to_string(_port);
        if (zmq_bind(_publisher, endpoint.c_str()) != 0) {
            Log::error("Could not bind to %s", endpoint.c_str());    
        } else { // successful binding
            _connected = true;
        }
    }


    Server::~Server() {
        zmq_close(_publisher);
        zmq_ctx_destroy(_context);
    }


    void Server::sleep_inner() {
    }


    void Server::sleep_outer() {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }


    bool Server::run() {
        if (!_connected) {
            return false;
        }

        while (!_stopped) {
            IMsgQueue::Messages popped;
            while (_msgQueue->pop(popped)) {
                bool rc = process(std::move(popped));
                if (!rc) {
                    return false;
                }

                sleep_inner();
            }

            sleep_outer();
        }

        return true;
    }


    bool Server::process(IMsgQueue::Messages&& popped) {
        for (auto&& msg : popped.msgs) {
            auto topicID = toLittleEndian(popped.topicID);
            sendBytes(&topicID[0], topicID.size(), true);

            auto&& container = std::get<0>(msg);

            void *memOwner = nullptr;
            auto customFree = std::get<1>(msg);
            if (customFree != nullptr) {
                memOwner = container;
            }

            if (!cryptAndSendBytes(container->getBufferPointer(),
                                   customFree,
                                   memOwner,
                                   container->getSize(),
                                   container->key())) {
                Log::error("Error during message transmission");
            }
        }

        return true;
    }


    void Server::stop() {
        Log::info("Stopping server ...");
        _stopped = true;
    }
}

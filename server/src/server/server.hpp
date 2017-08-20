#pragma once

#include <cassert>
#include <chrono>
#include <thread>

#include <zmq.h>

#include "config.hpp"
#include "crypto.hpp"

#include "SimpleLogger/logger.hpp"


namespace server {
    
    extern void customFree(void * /*data*/, void * /*hint*/);

    /*
     * T has to provide the implementations of the member function:
     *
     * std::vector<S> pop()
     *
     * whereas S has to provide the implementations of the member functions:
     *
     * S'  GetBufferPointer()
     * S'' GetSize()
     *
     * where the return values S' and S'' are convertible to byte * and
     * std::size_t, respectively.
     *
     * After poping instances of S, Server will sent these encrypted over the
     * wire and 
     *
     * (!) DELETE THEM AFTERWARDS (!)
     *
     * by calling 
     *
     * void customFree(void *s', void *t)
     *
     * s' and t are void pointers to respective S' and T instance, that was
     * poped before.
     * Not implementing this deallocation will lead dangling pointers!
     * It is reasonable to implement this custom deallocation near by the
     * implementation of T.
     */
    template<class T>
    class Server {
    private:
        bool _connected;
        bool _stopped;
        const port_t _port;
        const std::size_t _threads;

        void *_context;
        void *_publisher;

        T _dataQueue;


        bool sendMessage(zmq_msg_t *msg, const std::size_t size, bool more) {
            int rc = zmq_msg_send(msg, _publisher, more ? ZMQ_SNDMORE : 0);
            if (static_cast<std::size_t>(rc) != size) {
                return false;
            }

            return true;
        }

        bool sendBytes(byte *bytes, const std::size_t size, bool more) {
            zmq_msg_t msg;

            int rc = zmq_msg_init_size(&msg, size);
            if (rc != 0) {
                return false;
            }

            memcpy(zmq_msg_data(&msg), bytes, size);

            return sendMessage(&msg, size, more);
        }

        bool sendBytes(byte *bytes,
                       void *memOwner,
                       const std::size_t size,
                       bool more) {
            zmq_msg_t msg;
            
            int rc = zmq_msg_init_data(&msg, bytes, size, customFree, memOwner);
            if (rc != 0) {
                return false;
            }

            return sendMessage(&msg, size, more);
        }


        bool cryptAndSendBytes(byte *bytes, 
                               void *memOwner,
                               const std::size_t size,
                               const crypto::Key& key,
                               bool more = false) {
            auto iv = crypto::encrypt(bytes, size, key);

            /*
             * zeromq allocates memory on the heap, when 'size' is above some
             * threshold. In this case it is faster to have the data dynamically
             * allocated, too and save the memcpy step.
             * Typically, IV is below threshold, therefore the technique using
             * memcpy is faster / non-critically.
             */
            return sendBytes(&iv[0], crypto::IV_BLOCKSIZE, true) &&
                   sendBytes(bytes, memOwner, size, more);
        }


    public:
        Server(const Server&) = delete;

        Server& operator=(const Server&) = delete;

        Server(port_t port, std::size_t threads):
            _connected(false),
            _stopped(false),
            _port(port),
            _threads(threads) {
            _context = zmq_ctx_new();
            _publisher = zmq_socket(_context, ZMQ_PUB);

            std::string endpoint = std::string("tcp://*:")
                                 + std::to_string(_port);
            if (zmq_bind(_publisher, endpoint.c_str()) != 0) {
                Log::error("Could not bind to %s", endpoint.c_str());    
            } else { // successful binding
                _connected = true;
            }
        }

        virtual ~Server() {
            zmq_close(_publisher);
            zmq_ctx_destroy(_context);
        }

        bool run() {
            if (!_connected) {
                return false;
            }

            auto sleep = []() {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1s);
            };

            constexpr crypto::Key KEY {
                0x00, 0x01, 0x02, 0x03,
                0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b,
                0x0c, 0x0d, 0x0e, 0x0f
            };

            while (!_stopped) {
                for (auto container : _dataQueue.pop()) {

                    auto bytes = static_cast<byte *>(
                        container->GetBufferPointer()
                    );
                    auto size = static_cast<std::size_t>(
                        container->GetSize()
                    );

                    if (!cryptAndSendBytes(bytes, container, size, KEY)) {
                        Log::error("Error during message transmission");
                    }
                }

                sleep();
            }

            return true;
        }

        void stop() {
            Log::info("Stopping server...");
            _stopped = true;
        }
    };
}

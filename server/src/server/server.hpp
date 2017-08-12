#pragma once

#include <cassert>
#include <chrono>
#include <thread>

#include <zmq.h>

#include "config.hpp"
#include "logger.hpp"


namespace server {
    class Server {
    private:
        bool _connected;
        bool _stopped;
        const port_t _port;
        const std::size_t _threads;

        void *_context;
        void *_publisher;


        bool sendBytes(void *bytes, const std::size_t size, bool more = false) {
            zmq_msg_t msg;

            if (zmq_msg_init_size(&msg, size) != 0) {
                return false;
            }

            memcpy(zmq_msg_data(&msg), bytes, size);

            {
                int rc = zmq_msg_send(&msg, _publisher, more ? ZMQ_SNDMORE : 0);
                if (static_cast<std::size_t>(rc) != size) {
                    return false;
                }
            }

            return true;
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

            std::string endpoint = std::string("tcp://*:") + std::to_string(_port);
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

            while (!_stopped) {
                char msgdata[] = {'h', 'e', 'l', 'l', 'o'};
                if (!sendBytes(msgdata, 5)) {
                    Log::error("Error during message transmission");
                }

                {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1s);
                }
            }

            return true;
        }

        void stop() {
            Log::info("Stopping server...");
            _stopped = true;
        }
    };
}

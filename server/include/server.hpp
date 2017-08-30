#pragma once

#include <memory>
#include <vector>

#include <zmq.h>

#include "crypto.hpp"


namespace server {
    using CustomFreePtr_t = void (*)(void *data, void *hint);

    class IMessage {
    public:
        virtual byte *getBufferPointer() const = 0;
        virtual std::size_t getSize() const = 0;
        virtual const crypto::Key& key() const = 0;
    };

    class IMsgQueue {
    public:
        struct Messages {
            using msg_t = std::pair<IMessage *, CustomFreePtr_t>;

            std::size_t topicID;
            std::vector<msg_t> msgs;

            Messages(std::size_t id, std::vector<msg_t>&& m):
                topicID(id),
                msgs(std::forward<std::vector<msg_t>>(m)) {
            }
        };

        virtual ~IMsgQueue() = default;

        virtual Messages pop() = 0;
    };

    
    /*
     * After poping messages, Server will encrypt and send them over the wire.
     * For encryption Server use the provided key and generated a random IV.
     * The IV is sent firstly, followed by the corresponding encrypted message.
     * If messages are marked by setting the function pointer to some different
     * value than nullptr, Server will
     *
     * (!) DELETE THEM AFTERWARDS (!)
     *
     * by calling this custom free function.
     * Note, that passing nullptr, will result in the potentially time-consuming
     * call to memcpy, to copy the bytes to zmq.
     * Make sure to manually deallocate the passed pointer afterwards.
     *
     * The two void pointers 'data' and 'hint', point to the transferred bytes
     * and the IMessage instance, respectively.
     * The design of raw function pointers and void pointers originates from the
     * zmq API.
     */
    class Server {
    private:
        bool _connected;
        bool _stopped;
        const port_t _port;

        void *_context;
        void *_publisher;

        std::shared_ptr<IMsgQueue> _msgQueue;

        bool sendMessage(zmq_msg_t *, const std::size_t size, bool more);

        bool sendBytes(byte *, const std::size_t size, bool more);

        bool sendBytes(byte *,
                       CustomFreePtr_t,
                       void *memOwner,
                       const std::size_t size,
                       bool more);

        bool cryptAndSendBytes(byte *, 
                               CustomFreePtr_t,
                               void *memOwner,
                               const std::size_t size,
                               const crypto::Key& key,
                               bool more = false);


    public:
        Server(const Server&) = delete;

        Server& operator=(const Server&) = delete;

        Server(port_t, std::shared_ptr<IMsgQueue>);

        virtual ~Server();

        bool run();

        void stop();
    };
}

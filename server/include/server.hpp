#pragma once

#include <memory>
#include <vector>

#include <zmq.h>

#include "crypto.hpp"


namespace server {
    class IMessage {
    public:
        virtual byte *getBufferPointer() const = 0;
        virtual std::size_t getSize() const = 0;
        virtual const crypto::Key& key() const = 0;
    };

    class IMsgQueue {
    public:
        struct Messages {
            /*
             * The bool flag mark a message as candidate for deletion after
             * sending.
             * For more details, see class Server.
             */
            using msg_t = std::pair<IMessage *, bool>;

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

    
    extern void customFree(void * /*data*/, void * /*hint*/);

    /*
     * After poping messages, Server will encrypt and send these over the wire.
     * For encryption Server use the provided key and generated a random IV.
     * The IV is sent firstly, followed by the corresponding encrypted message.
     * If messages are marked by setting the bool flag (see struct Messages),
     * Server will
     *
     * (!) DELETE THEM AFTERWARDS (!)
     *
     * by calling 
     *
     * void customFree(void *s, void *t)
     *
     * s and t are void pointers to byte and T instance.
     * Not implementing this deallocation will lead dangling pointers!
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
                       void *memOwner,
                       const std::size_t size,
                       bool more);

        bool cryptAndSendBytes(byte *, 
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

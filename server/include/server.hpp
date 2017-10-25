#pragma once

#include <memory>
#include <vector>

#include <zmq.h>

#include "crypto.hpp"
#include "runnable.hpp"


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

            std::uint32_t topicID;
            std::vector<msg_t> msgs;

            Messages(): topicID(0) {
            }

            Messages(Messages&& other):
                topicID(std::move(other.topicID)), 
                msgs(std::move(other.msgs)) {
            }

            Messages& operator=(Messages&& other) {
                topicID = std::move(other.topicID);
                msgs = std::move(other.msgs);
                return *this;
            }

            Messages(std::size_t id, std::vector<msg_t>&& m):
                topicID(id),
                msgs(std::forward<std::vector<msg_t>>(m)) {
            }
        };

        virtual ~IMsgQueue() = default;

        virtual void push(Messages&&) = 0;

        virtual bool pop(Messages&) = 0;
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
    class Server: public utils::Runnable {
    private:
        bool _connected;
        const port_t _port;

        void *_context;
        void *_publisher;

        std::shared_ptr<IMsgQueue> _msgQueue;

        virtual bool sendMessage(zmq_msg_t *,
                                 const std::size_t size,
                                 bool more);

        virtual bool sendBytes(byte *, const std::size_t size, bool more);

        virtual bool sendBytes(byte *,
                       CustomFreePtr_t,
                       void *memOwner,
                       const std::size_t size,
                       bool more);

        virtual bool cryptAndSendBytes(byte *, 
                               CustomFreePtr_t,
                               void *memOwner,
                               const std::size_t size,
                               const crypto::Key& key,
                               bool more = false);
        
        virtual void sleep();


    public:
        Server(const Server&) = delete;

        Server& operator=(const Server&) = delete;

        Server(port_t, std::shared_ptr<IMsgQueue>);

        virtual ~Server();

        virtual bool run() override;

        virtual bool loop() override;

        virtual bool process(IMsgQueue::Messages&&);

        virtual void stop() override;

        template <typename T>
        static std::vector<byte> toLittleEndian(const T& x) {
            constexpr std::size_t nbytes = sizeof(x);
            std::vector<byte> bytes(nbytes);
            for (std::size_t i = 0; i < nbytes; ++i) {
                auto index = nbytes - 1 - i;
                bytes[index] = (x >> (i * 8));
            }

            return bytes;
        }
    };
}

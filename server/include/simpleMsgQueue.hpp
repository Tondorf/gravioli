#pragma once

#include <mutex>
#include <queue>

#include "server.hpp"


namespace server {
    class Message: public server::IMessage {
    protected:
        mutable byte *_data;
        const crypto::Key _key;
        const std::size_t _size;

    public:
        Message(const crypto::Key&, byte *, std::size_t size);

        virtual ~Message();

        virtual byte *getBufferPointer() const;

        virtual std::size_t getSize() const;

        virtual const crypto::Key& key() const;
    };


    class SimpleMsgQueue: public IMsgQueue {
    protected:
        std::mutex _mutex;
        std::queue<Messages> _queue;
        
    public:
        SimpleMsgQueue();

        virtual ~SimpleMsgQueue() = default;

        void push(Messages&&);

        bool pop(Messages&);
    };
}

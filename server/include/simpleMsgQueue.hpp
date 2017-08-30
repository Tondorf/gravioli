#pragma once

#include <atomic>
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
        std::mutex _lock;
        std::queue<Messages> _queue;
        
    public:
        static constexpr std::size_t
        SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD = 10;

        static std::atomic<std::size_t> currentlyAllocatedInstances;

        SimpleMsgQueue();

        virtual ~SimpleMsgQueue();

        void push(Messages&&);

        bool pop(Messages&);
    };
}

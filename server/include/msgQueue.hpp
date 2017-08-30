#pragma once

#include <atomic>

#include "server.hpp"

#include "flatbuffers/flatbuffers.h"


namespace server {
    class Message: public IMessage, 
                   public flatbuffers::FlatBufferBuilder {
    private:
        const crypto::Key _key;

    public:
        Message(const crypto::Key&);

        Message(const crypto::Key&, std::size_t allocSize);

        virtual ~Message() = default;

        virtual byte *getBufferPointer() const;

        virtual std::size_t getSize() const;

        virtual const crypto::Key& key() const;

        virtual flatbuffers::FlatBufferBuilder *asFlatBufferBuilder();
    };


    class MsgQueue: public IMsgQueue {
    private:
        int _lastAllocSize; 

    public:
        static constexpr std::size_t
        SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD = 10;

        static std::atomic<std::size_t> currentlyAllocatedInstances;

        MsgQueue(); 

        virtual ~MsgQueue();

        Messages pop();
    };

    void customFree(void *data, void *hint);
}

#include <atomic>
#include <chrono>
#include <thread>

#include "msgQueue.hpp"

#include "SimpleLogger/logger.hpp"

#include "planet_generated.h"


namespace server {
    Message::Message(const crypto::Key& key):
        flatbuffers::FlatBufferBuilder(),
        _key(key) {
    }


    Message::Message(const crypto::Key& key, std::size_t allocSize):
        flatbuffers::FlatBufferBuilder(allocSize),
        _key(key) {
    }


    byte *Message::getBufferPointer() const {
        return static_cast<byte *>(GetBufferPointer());
    }


    std::size_t Message::getSize() const {
        return static_cast<std::size_t>(GetSize());
    }


    const crypto::Key& Message::key() const {
        return _key;
    }


    flatbuffers::FlatBufferBuilder *Message::asFlatBufferBuilder() {
        return static_cast<FlatBufferBuilder *>(this);
    }


    MsgQueue::MsgQueue() : _lastAllocSize(-1) {
    }


    MsgQueue::~MsgQueue() {
        using namespace std::chrono_literals;

        while (currentlyAllocatedInstances.load() > 0) {
            Log::info("Waiting until all allocated instances of MsgQueue"
                      "are deleted: %d instances",
                      currentlyAllocatedInstances.load());

            std::this_thread::sleep_for(1s);
        }

        Log::info("All allocated instances of MsgQueue are deleted.");
    }


    IMsgQueue::Messages MsgQueue::pop() {
        if (currentlyAllocatedInstances.load() 
            > SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD) {
            Log::error("Number of simultaneously allocated instances "
                       "is above threshold: %d instances",
                       currentlyAllocatedInstances.load());
        }

        constexpr crypto::Key KEY {
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f
        };

        Message *builder = nullptr;
        if (_lastAllocSize > 0) {
            builder = new Message(KEY, _lastAllocSize);
        } else {
            builder = new Message(KEY);
        }

        ++currentlyAllocatedInstances;

        auto game = game::CreatePlanet(*(builder->asFlatBufferBuilder()),
                                       1.f, 2.f);
        builder->Finish(game);

        _lastAllocSize = static_cast<int>(builder->GetSize());

        const std::size_t ID = 0;
        std::vector<Messages::msg_t> msgs;
        msgs.push_back(std::make_pair(builder, true));

        return Messages(ID, std::move(msgs));
    }


    std::atomic<std::size_t> MsgQueue::currentlyAllocatedInstances(0);


    void customFree(void * /*data*/, void *hint) {
        --MsgQueue::currentlyAllocatedInstances;

        delete static_cast<Message *>(hint);
    }
}

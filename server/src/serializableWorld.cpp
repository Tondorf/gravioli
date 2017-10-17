#include <thread>

#include "SimpleLogger/logger.hpp"

#include "planet_generated.h"

#include "fbmessage.hpp"
#include "simpleMsgQueue.hpp"

#include "serializableWorld.hpp"



namespace simulation {
    std::atomic<std::size_t>
    SerializableWorld::currentlyAllocatedMsgInstances(0);


    void customFree(void *, void *);


    SerializableWorld::SerializableWorld(int id,
                                         const WorldProperties& wprop,
                                         MsgQueue_ptr msgQueue) :
        World(id, wprop),
        _msgQueue(msgQueue),
        _lastAllocSize(-1) {
    }


    SerializableWorld::~SerializableWorld() {
        using namespace std::chrono_literals;

        while (currentlyAllocatedMsgInstances.load() > 0) {
            Log::info("World #%d: Waiting until all allocated Msg instances "
                      "are deleted: %d instances",
                      currentlyAllocatedMsgInstances.load(), ID);

            std::this_thread::sleep_for(1s);
        }

        Log::info("World #%d: All allocated Msg instances are deleted.", ID);
    }


    void SerializableWorld::init(std::vector<SerializableWorld_ptr>& worlds,
                                 const WorldProperties& wprop,
                                 MsgQueue_ptr msgQueue) {
        worlds.push_back(
            std::make_shared<SerializableWorld>(0, wprop, msgQueue)
        );
    }


    void SerializableWorld::loop(std::uint64_t t) {
        World::loop(t);
        sendWorldToMsgQueue();
    }


    void SerializableWorld::sendWorldToMsgQueue() {
        {
            using namespace std::chrono_literals;
            auto n = currentlyAllocatedMsgInstances.load();
            while (n > MAX_ALLOCATED_MSG_INSTANCES) {
                Log::error("World #%d: Currently allocated Msg instances "
                           "above threshold: %d", ID, n);
                std::this_thread::sleep_for(10ms);
                n = currentlyAllocatedMsgInstances.load();
            }
        }

        flatbuffers::FlatBufferBuilder *builder = [](int size) {
            if (size > 0) {
                return new flatbuffers::FlatBufferBuilder(size);
            }
            return new flatbuffers::FlatBufferBuilder();
        }(_lastAllocSize);

        auto game = game::CreatePlanet(*builder, 1.f, 2.f);

        builder->Finish(game);

        const auto size = builder->GetSize();
        _lastAllocSize = size;

        constexpr crypto::Key key {
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f
        };
        std::size_t topicID = 0;

        auto allocNewMsg = [](const crypto::Key& key,
                              flatbuffers::FlatBufferBuilder *builder) {
            ++SerializableWorld::currentlyAllocatedMsgInstances;
            return new server::FBMessage(key, builder);
        };
        server::IMsgQueue::Messages msgs(topicID, {
            std::make_pair(allocNewMsg(key, builder), &customFree)
        });

        _msgQueue->push(std::move(msgs));
    }


    void customFree(void * /*data*/, void *hint) {
        --SerializableWorld::currentlyAllocatedMsgInstances;

        delete static_cast<server::FBMessage *>(hint);
    }
}

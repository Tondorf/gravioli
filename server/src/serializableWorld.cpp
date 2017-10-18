#include <algorithm>
#include <thread>

#include "SimpleLogger/logger.hpp"

#include "world_generated.h"

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
        _lastAllocSize(-1),
        _rndgen(_rnddev()) {
     }


    SerializableWorld::~SerializableWorld() {
        server::IMsgQueue::Messages popped;
        while (_msgQueue->pop(popped)) {
            for (auto&& msg : popped.msgs) {
                auto customFree = std::get<1>(msg);
                if (customFree != nullptr) {
                    void *memOwner = std::get<0>(msg);
                    customFree(nullptr, memOwner);
                }
            }
        }

        auto n = currentlyAllocatedMsgInstances.load();
        if (n > 0) {
            Log::error("World #%d: Found %d dangling messages. "
                       "This is a memory leak!", ID, n);
        } else {
            Log::info("World #%d: All allocated messages are deleted.", ID);
        }
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
                Log::error("World #%d: Currently allocated message instances "
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

        std::vector<flatbuffers::Offset<game::Planet>> planets;
        planets.reserve(_planets.size());
        for (auto&& p: _planets) {
            auto x = static_cast<float>(p.pos[0]);
            auto y = static_cast<float>(p.pos[1]);
            auto z = static_cast<float>(p.pos[2]);
            planets.push_back(
                game::CreatePlanet(*builder, x, y, z)
            );
        }
        shuffle(planets.begin(), planets.end(), _rndgen);

        auto game = game::CreatePlanets(
            *builder,
            builder->CreateVector(planets)
        );
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
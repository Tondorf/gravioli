#include "serializableWorld.hpp"
#include <chrono>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include "SimpleLogger/logger.hpp"
#include "config.hpp"
#include "crypto.hpp"
#include "fbmessage.hpp"
#include "flatbuffers/flatbuffers.h"
#include "player.hpp"
#include "playerProvider.hpp"
#include "server.hpp"


namespace simulation {
    std::atomic<std::size_t>
    SerializableWorld::currentlyAllocatedMsgInstances(0);


    void customFree(void *, void *);


    SerializableWorld::SerializableWorld(
        int id,
        std::shared_ptr<PlayerProvider> playerProvider,
        MsgQueue_ptr msgQueue) :

        World(id, playerProvider),
        _msgQueue(msgQueue),
        _lastAllocSize(-1) {
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

        auto allocNewMsg = [](const crypto::Key& key,
                              flatbuffers::FlatBufferBuilder *builder) {
            ++SerializableWorld::currentlyAllocatedMsgInstances;
            return new server::FBMessage(key, builder);
        };
        for (auto&& player : _playerProvider->getPlayers()) {
            flatbuffers::FlatBufferBuilder *builder = [](int size) {
                if (size > 0) {
                    return new flatbuffers::FlatBufferBuilder(size);
                }
                return new flatbuffers::FlatBufferBuilder();
            }(_lastAllocSize);

            serializeWorldForPlayer(std::move(player), builder);

            const auto size = builder->GetSize();
            _lastAllocSize = size;

            const auto&& topicID = player->getID();
            auto&& key = player->getKey();

            server::IMsgQueue::Messages msgs(topicID, {
                std::make_pair(allocNewMsg(key, builder), &customFree)
            });

            _msgQueue->push(std::move(msgs));
        }
    }


    void customFree(void * /*data*/, void *hint) {
        --SerializableWorld::currentlyAllocatedMsgInstances;

        delete static_cast<server::FBMessage *>(hint);
    }
}

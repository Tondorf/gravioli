#include <thread>

#include "config.hpp"
#include "fbmessage.hpp"
#include "simpleMsgQueue.hpp"
#include "server.hpp"

#include "planet_generated.h"
#include "SimpleLogger/logger.hpp"

#include "world.hpp"


namespace simulation {
    std::atomic<std::size_t> World::currentlyAllocatedMsgInstances(0);


    void customFree(void *, void *);


    World::World(int id,
                 const WorldInfoProvider& winfo,
                 std::shared_ptr<server::IMsgQueue> msgQueue) :
        _winfo(winfo),
        _msgQueue(msgQueue),
        _stopped(false),
        _lastAllocSize(-1),
        ID(id) {
        Log::debug("Creating new world: #%d", id);
    }


    World::~World() {
        using namespace std::chrono_literals;

        while (currentlyAllocatedMsgInstances.load() > 0) {
            Log::info("World #%d: Waiting until all allocated Msg instances "
                      "are deleted: %d instances",
                      currentlyAllocatedMsgInstances.load(), ID);

            std::this_thread::sleep_for(1s);
        }

        Log::info("World #%d: All allocated Msg instances are deleted.", ID);
    }


    void World::init(std::vector<std::shared_ptr<World>>& worlds,
                     const WorldInfoProvider& winfo,
                     std::shared_ptr<server::IMsgQueue> msgQueue) {
        worlds.push_back(std::make_shared<World>(0, winfo, msgQueue));
    }


    void World::sleep() {
        std::this_thread::sleep_for(WORLD_SLEEP);
    }


    bool World::run() {
        while (!_stopped) {
            sendWorldToMsgQueue();
            sleep();
        }

        return true;
    }


    void World::stop() {
        _stopped = true;
    }


    void World::sendWorldToMsgQueue() {
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
            ++World::currentlyAllocatedMsgInstances;
            return new world::FBMessage(key, builder);
        };
        server::IMsgQueue::Messages msgs(topicID, {
            std::make_pair(allocNewMsg(key, builder), &customFree)
        });

        _msgQueue->push(std::move(msgs));
    }


    void customFree(void * /*data*/, void *hint) {
        --World::currentlyAllocatedMsgInstances;

        delete static_cast<world::FBMessage *>(hint);
    }
}

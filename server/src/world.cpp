#include <thread>

#include "config.hpp"
#include "server.hpp"

#include "planet_generated.h"
#include "SimpleLogger/logger.hpp"

#include "world.hpp"


namespace simulation {
    void customFree(void * /*data*/, void *hint) {
        --World::currentlyAllocatedFBInstances;

        delete static_cast<flatbuffers::FlatBufferBuilder *>(hint);
    }


    World::World(int id,
                 const WorldInfoProvider& winfo,
                 std::shared_ptr<server::IMsgQueue> msgQueue) :
        _winfo(winfo),
        _msgQueue(msgQueue),
        _stopped(false),
        _lastAllocSize(-1),
        ID(id) {
    }


    World::~World() {
        using namespace std::chrono_literals;

        while (currentlyAllocatedFBInstances.load() > 0) {
            Log::info("Waiting until all allocated FB instances \
                       are deleted: %d instances",
                      currentlyAllocatedFBInstances.load());

            std::this_thread::sleep_for(1s);
        }

        Log::info("All allocated FB instances are deleted.");
    }


    void World::init(std::vector<World>& worlds,
                     const WorldInfoProvider& winfo,
                     std::shared_ptr<server::IMsgQueue> msgQueue) {
        worlds.push_back(World(0, winfo, msgQueue));
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
        flatbuffers::FlatBufferBuilder *builder;
        if (_lastAllocSize > 0) {
            builder = new flatbuffers::FlatBufferBuilder(_lastAllocSize);
        } else {
            builder = new flatbuffers::FlatBufferBuilder();
        }
        ++currentlyAllocatedFBInstances;

        auto game = game::CreatePlanet(*builder, 1.f, 2.f);

        builder->Finish(game);
        _lastAllocSize = static_cast<int>(builder->GetSize());

        auto *data = builder->GetBufferPointer();
        auto size = static_cast<std::size_t>(builder->GetSize());

        std::size_t topicID = 0;
        server::IMsgQueue::Messages msg(topicID, {
            std::make_pair(data, &customFree)
        });

        std::vector<server::IMsgQueue::Messages> msgs {
            std::move(msg)
        };

        _msgQueue->push(std::move(msgs));
    }
}

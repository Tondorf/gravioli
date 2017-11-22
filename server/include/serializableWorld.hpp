#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>
#include "playerProvider.hpp"
#include "world.hpp"
namespace flatbuffers { class FlatBufferBuilder; }
namespace server { class IMsgQueue; }
namespace simulation { class Player; }

namespace simulation {
    class SerializableWorld: public World {
    private:
        using SerializableWorld_ptr = std::shared_ptr<SerializableWorld>;
        using MsgQueue_ptr = std::shared_ptr<server::IMsgQueue>;

        MsgQueue_ptr _msgQueue;
        int _lastAllocSize;

        static std::atomic<std::size_t> currentlyAllocatedMsgInstances;

        virtual void sendWorldToMsgQueue() override;

        friend void customFree(void *, void *);

    protected:
        virtual void serializeWorldForPlayer(
            std::shared_ptr<Player>,
            flatbuffers::FlatBufferBuilder *
        ) = 0;

    public:
        SerializableWorld(int id,
                          std::shared_ptr<PlayerProvider>,
                          MsgQueue_ptr msgQueue);

        virtual ~SerializableWorld();

        template <class T>
        static void init(std::vector<std::shared_ptr<T>>& worlds,
                         MsgQueue_ptr msgQueue) {
            static_assert(
                std::is_base_of<SerializableWorld, T>::value,
                "T must be a descendant of simulation::SerializableWorld"
            );

            auto newWorld = [&msgQueue](int id) {
                auto&& playerProvider = std::make_shared<PlayerProvider>();
                return std::make_shared<T>(id, playerProvider, msgQueue);
            };
            worlds.push_back(newWorld(0));
        }
    };
}

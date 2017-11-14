#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "world.hpp"
namespace server { class IMsgQueue; }
namespace simulation { class PlayerProvider; }


namespace simulation {
    class SerializableWorld: public World {
    private:
        using SerializableWorld_ptr = std::shared_ptr<SerializableWorld>;
        using MsgQueue_ptr = std::shared_ptr<server::IMsgQueue>;

        MsgQueue_ptr _msgQueue;
        int _lastAllocSize;

        static std::atomic<std::size_t> currentlyAllocatedMsgInstances;

        virtual void loop(std::uint64_t) override;

        void sendWorldToMsgQueue();

        friend void customFree(void *, void *);

    public:
        SerializableWorld(int id,
                          std::shared_ptr<PlayerProvider>,
                          MsgQueue_ptr msgQueue);

        virtual ~SerializableWorld();

        static void init(std::vector<SerializableWorld_ptr>&, MsgQueue_ptr);
    };
}

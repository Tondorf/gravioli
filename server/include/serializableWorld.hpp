#pragma once

#include <atomic>

#include "world.hpp"


namespace simulation {
    class SerializableWorld: public World {
    private:
        using SerializableWorld_ptr = std::shared_ptr<SerializableWorld>;
        using MsgQueue_ptr = std::shared_ptr<server::IMsgQueue>;

        MsgQueue_ptr _msgQueue;
        int _lastAllocSize;

        static std::atomic<std::size_t> currentlyAllocatedMsgInstances;

        void loop(std::uint64_t) override;

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

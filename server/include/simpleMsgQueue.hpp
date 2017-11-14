#pragma once

#include <mutex>
#include <queue>
#include "server.hpp"


namespace server {
    class SimpleMsgQueue: public IMsgQueue {
    protected:
        std::mutex _mutex;
        std::queue<Messages> _queue;
        
    public:
        SimpleMsgQueue();

        virtual ~SimpleMsgQueue() = default;

        virtual void push(Messages&&) override;

        virtual bool pop(Messages&) override;
    };
}

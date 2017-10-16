#include "simpleMsgQueue.hpp"

#include "config.hpp"


namespace server {
    SimpleMsgQueue::SimpleMsgQueue() {
    }


    void SimpleMsgQueue::push(Messages&& msgs) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        _queue.push(std::move(msgs));
    }


    bool SimpleMsgQueue::pop(Messages& msgs) {
        std::lock_guard<std::mutex> lock(_mutex);

        bool success = false;

        if (!_queue.empty()) {
            msgs = std::move(_queue.front());
            _queue.pop();
            success = true;
        }

        return success;
    }
}

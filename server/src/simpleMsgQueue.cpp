#include <chrono>
#include <thread>

#include "simpleMsgQueue.hpp"

#include "config.hpp"

#include "SimpleLogger/logger.hpp"


namespace server {
    Message::Message(const crypto::Key& key, byte *data, std::size_t size):
        _data(data),
        _key(key),
        _size(size) {
    }


    byte *Message::getBufferPointer() const {
        return _data;
    }


    std::size_t Message::getSize() const {
        return _size;
    }


    const crypto::Key& Message::key() const {
        return _key;
    }


    Message::~Message() {
        delete[] _data;
        _data = nullptr;
    }


    SimpleMsgQueue::SimpleMsgQueue() {
    }


    SimpleMsgQueue::~SimpleMsgQueue() {
        using namespace std::chrono_literals;

        while (currentlyAllocatedInstances.load() > 0) {
            Log::info("Waiting until all allocated instances of MsgQueue"
                      "are deleted: %d instances",
                      currentlyAllocatedInstances.load());

            std::this_thread::sleep_for(100ms);
        }

        Log::info("All allocated instances of MsgQueue are deleted.");
    }


    void SimpleMsgQueue::push(Messages&& msgs) {
        _lock.lock();
        _queue.push(std::move(msgs));
        _lock.unlock();
    }


    bool SimpleMsgQueue::pop(Messages& msgs) {
        if (currentlyAllocatedInstances.load() 
            > SIMULTANEOUSLY_ALLOCATED_INSTANCES_THRESHOLD) {
            Log::error("Number of simultaneously allocated instances "
                       "is above threshold: %d instances",
                       currentlyAllocatedInstances.load());
        }

        bool success = false;

        _lock.lock();
        if (!_queue.empty()) {
            msgs = std::move(_queue.front());
            _queue.pop();
            success = true;
        }
        _lock.unlock();

        return success;
    }

    std::atomic<std::size_t> SimpleMsgQueue::currentlyAllocatedInstances(0);
}

#include "simpleMsgQueue.hpp"

#include "config.hpp"


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

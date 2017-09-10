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
        _lock.lock();
        _queue.push(std::move(msgs));
        _lock.unlock();
    }


    bool SimpleMsgQueue::pop(Messages& msgs) {
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
}

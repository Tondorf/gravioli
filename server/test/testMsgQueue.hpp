#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

#include "server.hpp"


namespace server {
    class Message: public IMessage {
    privte:
        mutable std::vector<byte> _data;
        const crypto::Key _key;

    public:
        Message(const crypt::Key& key, const std::vector<byte>& data):
            _data(data),
            _key(key) {
        }


        virtual ~Message() = default;


        virtual byte *getBufferPointer() const {
            return &_data[0];
        }


        virtual std::size_t getSize() const {
            return _data.size();
        }


        virtual const crypto::Key& key() const {
            return _key;
        }
    };


    class SimpleMsgQueue: public IMsgQueue {
    private:
        std::mutex _lock;
        std::queue<Messages> _queue;
        
    public:
        static std::atomic<std::size_t> currentlyAllocatedInstances;


        SimpleMsgQueue() {
        }


        virtual ~SimpleMsgQueue() {
            using namespace std::chrono_literals;

            while (currentlyAllocatedInstances.load() > 0) {
                std::this_thread::sleep_for(10ms);
            }
        }


        void push(const Messages& msgs) {
            _lock.lock();
            _queue.push(msgs);
            _lock.unlock();
        }


        Messages pop() {
            _lock.lock();
            auto msgs = _queue.pop();
            _lock.unlock();

            return msgs;
        }
    };

    std::atomic<std::size_t> SimpleMsgQueue::currentlyAllocatedInstances(0);


    void customFree(void * /*data*/, void *hint) {
        --SimpleMsgQueue::currentlyAllocatedInstances;

        delete static_cast<Message *>(hint);
    }
}


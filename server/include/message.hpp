#pragma once

#include <functional>
#include "server.hpp"


namespace server {
    template <typename T>
    class Message: public server::IMessage {
    protected:
        mutable T *_data;
        const crypto::Key _key;
        const std::size_t _size;
        std::function<void(T *)> _del;

    public:
        Message(const crypto::Key& key,
                T *data,
                std::size_t size,
                std::function<void(T *)> del):
            _data(data),
            _key(key),
            _size(size),
            _del(del) {
        }

        virtual ~Message() {
            _del(_data);
            _data = nullptr;
        }

        virtual byte *getBufferPointer() const = 0;

        virtual std::size_t getSize() const {
            return _size;
        }

        virtual const crypto::Key& key() const {
            return _key;
        }
    };
}

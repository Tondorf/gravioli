#pragma once

#include "message.hpp"


class ByteMessage: public server::Message<byte> {
public:
    ByteMessage(const crypto::Key& key, byte *data, const std::size_t size):
        server::Message<byte>(key, data, size, [](byte *bytes) {
            delete[] bytes;
        }) {
    }

    virtual ~ByteMessage() = default;

    virtual byte *getBufferPointer() const override {
        return server::Message<byte>::_data;
    }
};

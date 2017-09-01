#include "helper.hpp"


server::Message *createMsg(const crypto::Key& key,
                           const std::vector<byte>& data) {
    const std::size_t size = data.size();
    byte *bytes = new byte[size];
    for (std::size_t i = 0; i < size; ++i) {
        bytes[i] = data[i];
    }

    return new server::Message(key, bytes, size);
}

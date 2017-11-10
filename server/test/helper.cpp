#include "byteMessage.hpp"
#include "helper.hpp"


std::atomic<std::size_t> currentlyAllocatedMsgInstances(0);


ByteMessage *createMsg(const crypto::Key& key,
                           const std::vector<byte>& data) {
    const std::size_t size = data.size();
    byte *bytes = new byte[size];
    for (std::size_t i = 0; i < size; ++i) {
        bytes[i] = data[i];
    }

    return new ByteMessage(key, bytes, size);
}


void deleteMsg(void *, void *hint) {
    --currentlyAllocatedMsgInstances;

    delete static_cast<ByteMessage *>(hint);
}

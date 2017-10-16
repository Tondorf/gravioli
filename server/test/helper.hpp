#pragma once

#include <atomic>

#include "byteMessage.hpp"


extern std::atomic<std::size_t> currentlyAllocatedMsgInstances;

ByteMessage *createMsg(const crypto::Key&, const std::vector<byte>&);

void deleteMsg(void *data, void *hint);

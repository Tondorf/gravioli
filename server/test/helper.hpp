#pragma once

#include <atomic>
#include <cstddef>
#include <vector>
#include "config.hpp"
#include "crypto.hpp"
class ByteMessage;


extern std::atomic<std::size_t> currentlyAllocatedMsgInstances;

ByteMessage *createMsg(const crypto::Key&, const std::vector<byte>&);

void deleteMsg(void *data, void *hint);

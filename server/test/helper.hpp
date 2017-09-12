#pragma once

#include <atomic>

#include "server/simpleMsgQueue.hpp"


extern std::atomic<std::size_t> currentlyAllocatedMsgInstances;

server::Message *createMsg(const crypto::Key&, const std::vector<byte>&);

void deleteMsg(void *data, void *hint);

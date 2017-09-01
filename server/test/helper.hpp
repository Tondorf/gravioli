#pragma once

#include "simpleMsgQueue.hpp"


server::Message *createMsg(const crypto::Key&, const std::vector<byte>&);

#pragma once

#include "message.hpp"
#include "server.hpp"

#include "flatbuffers/flatbuffers.h"


namespace world {
    class FBMessage: public server::Message<flatbuffers::FlatBufferBuilder> {
    public:
        FBMessage(const crypto::Key&, flatbuffers::FlatBufferBuilder *builder);

        virtual ~FBMessage() = default;

        virtual byte *getBufferPointer() const override;
    };
}

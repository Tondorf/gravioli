#pragma once

#include "config.hpp"
#include "crypto.hpp"
#include "message.hpp"
namespace flatbuffers { class FlatBufferBuilder; }


namespace server {
    class FBMessage: public server::Message<flatbuffers::FlatBufferBuilder> {
    public:
        FBMessage(const crypto::Key&, flatbuffers::FlatBufferBuilder *builder);

        virtual ~FBMessage() = default;

        virtual byte *getBufferPointer() const override;
    };
}

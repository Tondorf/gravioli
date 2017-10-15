#pragma once

#include "server.hpp"

#include "flatbuffers/flatbuffers.h"


namespace world {
    class FBMessage: public server::IMessage {
    private:
        const crypto::Key _key;
        flatbuffers::FlatBufferBuilder *_builder;

    public:
        FBMessage(const crypto::Key&, flatbuffers::FlatBufferBuilder *);

        virtual ~FBMessage();

        virtual byte *getBufferPointer() const;

        virtual std::size_t getSize() const;

        virtual const crypto::Key& key() const;
    };
}

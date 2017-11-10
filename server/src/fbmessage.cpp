#include "fbmessage.hpp"
#include <functional>
#include "flatbuffers/flatbuffers.h"


namespace server {
    FBMessage::FBMessage(const crypto::Key& key,
                         flatbuffers::FlatBufferBuilder *builder):
        server::Message<flatbuffers::FlatBufferBuilder>(key, builder,
        builder->GetSize(),
        [](flatbuffers::FlatBufferBuilder *builder) {
            delete builder;
        }) {
    }


    byte *FBMessage::getBufferPointer() const {
        using fbb_t = flatbuffers::FlatBufferBuilder;
        return server::Message<fbb_t>::_data->GetBufferPointer();
    }
}

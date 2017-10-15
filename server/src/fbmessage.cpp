#include "fbmessage.hpp"

namespace world {
    FBMessage::FBMessage(const crypto::Key& key,
                         flatbuffers::FlatBufferBuilder *builder):
        _key(key),
        _builder(builder) {
    }


    FBMessage::~FBMessage() {
        delete _builder;
        _builder = nullptr;
    }


    byte *FBMessage::getBufferPointer() const {
        return _builder->GetBufferPointer();
    }


    std::size_t FBMessage::getSize() const {
        return _builder->GetSize();
    }


    const crypto::Key& FBMessage::key() const {
        return _key;
    }
}

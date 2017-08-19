#pragma once

#include "planet_generated.h"


namespace server {
    class Publisher {
    public:
        using BinaryData = flatbuffers::DetachedBuffer;

        std::vector<BinaryData> pop() {
            flatbuffers::FlatBufferBuilder builder;
            auto game = game::CreatePlanet(builder, 1.f, 2.f);
            builder.Finish(game);

            std::vector<BinaryData> data;
            data.push_back(builder.Release());

            return data;
        }
    };
}

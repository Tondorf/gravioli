#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>
#include "serializableWorld.hpp"
namespace flatbuffers { class FlatBufferBuilder; }
namespace server { class IMsgQueue; }
namespace server { class Player; }
namespace server { class PlayerProvider; }


namespace gravioli {
    using Vec3d = std::array<double, 3>;

    struct Planet {
        Vec3d pos;
    };

    class GravioliWorld : public server::SerializableWorld {
    private:
        std::random_device _rnddev;
        std::mt19937 _rndgen;

        std::vector<Planet> _planets;

        virtual void init();

        virtual void loop(std::uint64_t) override;

        virtual void serializeWorldForPlayer(
            std::shared_ptr<server::Player>,
            flatbuffers::FlatBufferBuilder *
        ) override;

    public:
        GravioliWorld() = delete;

        GravioliWorld(int id,
                      std::shared_ptr<server::PlayerProvider>,
                      std::shared_ptr<server::IMsgQueue>);

        GravioliWorld(const GravioliWorld&) = delete;

        GravioliWorld& operator=(const GravioliWorld&) = delete;

        virtual ~GravioliWorld() = default;
    };
}

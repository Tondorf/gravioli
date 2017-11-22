#include "gravioliWorld.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include "config.hpp"
#include "flatbuffers/flatbuffers.h"
#include "world_generated.h"
namespace server { class IMsgQueue; }
namespace simulation { class Player; }


namespace simulation {
    GravioliWorld::GravioliWorld(
        int id,
        std::shared_ptr<PlayerProvider> playerProvider,
        std::shared_ptr<server::IMsgQueue> msgQueue) :
        SerializableWorld(id, playerProvider, msgQueue),
        _rndgen(_rnddev()) {

        init();
    }


    void GravioliWorld::init() {
        auto newPlanet = [](double x, double y, double z) {
            Vec3d pos{x, y, z};
            Planet p;
            p.pos = pos;
            return p;
        };

        _planets.clear();
        _planets.push_back(newPlanet(0, 0, 0));
        _planets.push_back(newPlanet(0, 0, 0));
    }


    void GravioliWorld::loop(std::uint64_t t) {
        const auto n = _planets.size();

        constexpr std::size_t x = 0;
        constexpr std::size_t y = 1;
        constexpr std::size_t z = 2;
        for (std::size_t i = 0; i < n; ++i) {
            auto phase1 = static_cast<double>(t) / 2. * pi / 1000.;
            auto phase2 = static_cast<double>(t) / 2. * pi / 2000.;
            auto phase3 = static_cast<double>(t) / 2. * pi / 3000.;

            _planets[i].pos[x] = std::sin(phase1) + static_cast<double>(2 * i);
            _planets[i].pos[y] = std::sin(phase2);
            _planets[i].pos[z] = std::sin(phase3);
        }
    }


    void GravioliWorld::serializeWorldForPlayer(
        std::shared_ptr<Player> /*player*/,
        flatbuffers::FlatBufferBuilder *builder) {

        std::vector<flatbuffers::Offset<game::Planet>> planets;
        planets.reserve(_planets.size());
        for (auto&& p: _planets) {
            auto x = static_cast<float>(p.pos[0]);
            auto y = static_cast<float>(p.pos[1]);
            auto z = static_cast<float>(p.pos[2]);
            planets.push_back(
                game::CreatePlanet(*builder, x, y, z)
            );
        }
        shuffle(planets.begin(), planets.end(), _rndgen);

        auto game = game::CreatePlanets(
            *builder,
            builder->CreateVector(planets)
        );

        builder->Finish(game);
    }
}

#pragma once

#include <memory>
#include <string>
#include <map>


namespace world {
  struct MassiveSphere {
    double x;
    double y;
    double mass;
    double radius;
  };

  struct Planet : MassiveSphere {
    std::size_t id;
    double health;
  };

  struct Player {
    std::size_t id;
    std::string name;
    std::size_t skillpoints;
    std::shared_ptr<Planet> homebase;
    double aim;
    double cooldown;
  };

  struct Shot : MassiveSphere {
    std::size_t id;
    std::shared_ptr<Player> origin;
    double vx;
    double vy;
    double ttl;
    double dmg;
  };

  class World {
  private:
    std::map<std::size_t, std::shared_ptr<Planet>> _planets;
    std::map<std::size_t, std::shared_ptr<Shot>> _shots;
    std::map<std::size_t, std::shared_ptr<Player>> _players;

    World();

  public:
    virtual ~World();

    static World &getInstance();

    World(World const &) = delete;

    void operator=(World const &) = delete;

    bool addPlayer(std::size_t id);

    bool nextFreePlanet(std::size_t &id);

    const std::map<std::size_t, std::shared_ptr<Planet>> &getPlanets() const;

    const std::map<std::size_t, std::shared_ptr<Shot>> &getShots() const;

    const std::map<std::size_t, std::shared_ptr<Player>> &getPlayers() const;
  };
}

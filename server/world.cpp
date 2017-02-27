#include <cassert>
#include <cmath>
#include <set>

#include "world.h"

namespace world {
  World &World::getInstance() {
    static World instance;
    return instance;
  }

  World::World() : _nextShotID(0) {
    auto planet = std::make_shared<Planet>();
    planet->id = 0;
    planet->health = 1.;
    planet->x = 0.;
    planet->y = 0.;
    planet->mass = 1e3;
    planet->radius = 10.;

    _planets[planet->id] = planet;
  }

  World::~World() {
  }

  bool World::addPlayer(std::size_t id) {
    auto player = std::make_shared<Player>();
    player->id = id;
    player->name = "";
    player->skillpoints = 0;
    player->aim = 0.;
    player->cooldown = 0.;

    std::size_t pid;
    if (!nextFreePlanet(pid)) {
      return false;
    }
    assert(_planets.count(pid) == 1);
    player->homebase = _planets[pid];

    assert(_players.count(id) == 0);
    _players[id] = player;

    return true;
  };

  bool World::nextFreePlanet(std::size_t &id) {
    std::set<std::size_t> usedIDs;
    for (auto kv : _players) {
      usedIDs.insert(kv.second->id);
    }

    for (auto kv : _planets) {
      auto planet = kv.second;
      if (usedIDs.count(planet->id) == 0) {
        id = planet->id;
        return true;
      }
    }

    return false;
  }

  bool World::playerShoots(std::shared_ptr<Player> player) {
    if (player->cooldown > 0) {
      return false;
    }

    auto shot = std::make_shared<Shot>();
    std::size_t id = _nextShotID;
    _nextShotID = id + 1;
    shot->id = id;
    shot->origin = player;
    shot->vx = player->gunlength * sin(player->aim);
    shot->vy = player->gunlength * cos(player->aim);
    shot->ttl = 5.;
    shot->dmg = 1.;

    _shots[id] = shot;

    return true;
  }

  const std::map<std::size_t, std::shared_ptr<Planet>> &World::getPlanets() const {
    return _planets;
  }

  const std::map<std::size_t, std::shared_ptr<Shot>> &World::getShots() const {
    return _shots;
  }

  const std::map<std::size_t, std::shared_ptr<Player>> &World::getPlayers() const {
    return _players;
  }
}

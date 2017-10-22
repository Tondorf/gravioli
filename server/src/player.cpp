#include "player.hpp"


namespace simulation {
    Player::Player(std::uint64_t id, const crypto::Key& key):
        ID(id),
        _key(key) {
    }


    std::uint64_t Player::getID() const {
        return ID;
    }


    const crypto::Key& Player::getKey() const {
        return _key;
    }
}

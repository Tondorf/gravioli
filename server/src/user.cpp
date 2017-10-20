#include "user.hpp"


namespace simulation {
    User::User(std::uint64_t id, const crypto::Key& key):
        ID(id),
        _key(key) {
    }


    std::uint64_t User::getID() const {
        return ID;
    }


    const crypto::Key& User::getKey() const {
        return _key;
    }
}

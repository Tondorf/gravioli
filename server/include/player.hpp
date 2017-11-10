#pragma once

#include <cstdint>
#include "crypto.hpp"


namespace simulation {
    class Player {
    private:
        const std::uint64_t ID;
        const crypto::Key _key;

    public:
        Player(std::uint64_t, const crypto::Key&);

        virtual ~Player() = default;

        std::uint64_t getID() const;

        const crypto::Key& getKey() const;
    };
}

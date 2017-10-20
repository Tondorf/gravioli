#pragma once

#include "crypto.hpp"


namespace simulation {
    class User {
    private:
        const std::uint64_t ID;
        const crypto::Key _key;

    public:
        User(std::uint64_t, const crypto::Key&);

        virtual ~User() = default;

        std::uint64_t getID() const;

        const crypto::Key& getKey() const;
    };
}

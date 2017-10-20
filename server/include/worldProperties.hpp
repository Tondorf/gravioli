#pragma once

#include <memory>
#include <vector>

#include "user.hpp"


namespace simulation {
    class WorldProperties {
    private:
        std::vector<std::shared_ptr<User>> _users;

    public:
        WorldProperties();

        virtual ~WorldProperties() = default;

        void updateUsers();

        const std::vector<std::shared_ptr<User>>& getUsers();
    };
}

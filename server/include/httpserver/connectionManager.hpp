#pragma once

#include <set>

#include "connection.hpp"


namespace httpserver {
    class ConnectionManager {
    private:
        std::set<Connection_ptr> _connections;

    public:
        ConnectionManager();

        virtual ~ConnectionManager() = default;

        ConnectionManager(const ConnectionManager&) = delete;

        ConnectionManager& operator=(const ConnectionManager&) = delete;

        virtual void start(Connection_ptr);

        virtual void stop(Connection_ptr);

        virtual void stopAll();
    };
}

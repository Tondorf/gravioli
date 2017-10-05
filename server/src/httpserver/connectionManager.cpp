#include "httpserver/connectionManager.hpp"


namespace httpserver {
    ConnectionManager::ConnectionManager() {
    }


    void ConnectionManager::start(Connection_ptr c) {
        _connections.insert(c);
        c->start();
    }


    void ConnectionManager::stop(Connection_ptr c) {
        _connections.erase(c);
        c->stop();
    }


    void ConnectionManager::stopAll() {
        for (auto c: _connections) {
            c->stop();
        }

        _connections.clear();
    }
}

#pragma once

#include <string>

#include <boost/asio.hpp>

#include "config.hpp"

#include "httpserver/connection.hpp"
#include "httpserver/connectionManager.hpp"


namespace httpserver {
    template <class T>
    class Server {
        static_assert(
            std::is_base_of<IRequestHandler, T>::value,
            "T must be a descendant of IRequestHandler"
        );
    private:
        boost::asio::io_service _ios;
        boost::asio::ip::tcp::acceptor _acceptor;
        boost::asio::ip::tcp::socket _socket;
        ConnectionManager _connectionManager;
        T _requestHandler;


        virtual void accept() {
            _acceptor.async_accept(_socket,
                                   [this](boost::system::error_code ec) {
                if (_acceptor.is_open()) {
                    if (!ec) {
                        _connectionManager.start(std::make_shared<Connection>(
                            std::move(_socket),
                            _connectionManager,
                            _requestHandler
                        ));
                    }

                    accept();
                }
            });
        }

    public:
        Server(const std::string& address = "127.0.0.1",
               const port_t& port = 80) :
            _ios(),
            _acceptor(_ios),
            _socket(_ios),
            _connectionManager(),
            _requestHandler() {

            boost::asio::ip::tcp::resolver resolver(_ios);
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({
                address,
                std::to_string(port)
            });
            _acceptor.open(endpoint.protocol());
            _acceptor.set_option(
                boost::asio::ip::tcp::acceptor::reuse_address(true)
            );
            _acceptor.bind(endpoint);
            _acceptor.listen();

            accept();
        }

        virtual ~Server() = default;

        Server(const Server&) = delete;

        Server& operator=(const Server&) = delete;


        virtual bool run() {
            _ios.run();
            return true;
        }


        virtual void stop() {
            _acceptor.close();
            _connectionManager.stopAll();
        }
    };
}

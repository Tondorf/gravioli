#pragma once

#include <array>
#include <memory>

#include <boost/asio.hpp>

#include "httpserver/reply.hpp"
#include "httpserver/request.hpp"
#include "httpserver/requestParser.hpp"
#include "httpserver/requestHandler.hpp"


namespace httpserver {
    class ConnectionManager;

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        constexpr static std::size_t BUFFERSIZE = 8192;

    private:
        boost::asio::ip::tcp::socket _socket;
        ConnectionManager& _connectionManager;
        Request _request;
        RequestParser _requestParser;
        IRequestHandler& _requestHandler;
        Reply _reply;
        std::array<char, BUFFERSIZE> _buffer;

        virtual void read();

        virtual void write();

    public:
        Connection(const Connection&) = delete;

        virtual Connection& operator=(const Connection&) = delete;

        virtual ~Connection() = default;

        Connection(boost::asio::ip::tcp::socket socket,
                   ConnectionManager& connectionManager,
                   IRequestHandler& requestHandler);

        virtual void start();

        virtual void stop();
    };

    using Connection_ptr = std::shared_ptr<Connection>;
}

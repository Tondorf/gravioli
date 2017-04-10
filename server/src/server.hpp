#pragma once

#include <future>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include "beast/http.hpp"
#include "beast/core/handler_helpers.hpp"
#include "beast/core/handler_ptr.hpp"
#include "beast/core/placeholders.hpp"
#include "beast/core/streambuf.hpp"
#include "beast/core/to_string.hpp"

#include "acclogger.hpp"
#include "logger.hpp"


class Server {
private:
    boost::asio::io_service _ios;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _socket;
    std::vector<std::thread> _threads;

    class Peer : public std::enable_shared_from_this<Peer> {
    private:
        std::uint64_t _id;
        boost::asio::ip::tcp::socket _socket;
        Server& _server;
        boost::asio::io_service::strand _strand;
        beast::streambuf _sbuff;
        beast::http::request<beast::http::string_body>_reqtype;

    public:
        Peer(Peer&&) = default;

        Peer(const Peer &) = default;

        Peer& operator=(Peer&&) = delete;

        Peer& operator=(const Peer &) = delete;

        Peer(boost::asio::ip::tcp::socket&& socket, Server& server)
            : _socket(std::move(socket))
            , _server(server)
            , _strand(_socket.get_io_service()) {
            static std::uint64_t id = 0;
            _id = id;
            id += 1;
        }

        ~Peer() = default;

        void run() {
            read();
        }

        void read() {
            async_read(_socket, _sbuff, _reqtype, _strand.wrap(
                std::bind(&Peer::onRead, shared_from_this(), beast::asio::placeholders::error)));
        }

        void onRead(const boost::system::error_code& e) {
            if (e) {
                Log::error("Failure during reading (error code: %d)", e);
                return;
            }

            auto ip = _socket.remote_endpoint().address().to_string();
            auto type = _reqtype.method;
            auto url = _reqtype.url;

            beast::http::response<beast::http::string_body> res;
            try {
                if (url == "/" || url == "/index.html" || url == "/index.htm") {
                    res.status = 200;
                    res.reason = "OK";
                    res.version = _reqtype.version;
                    res.fields.insert("Content-Type", "text/html");
                    res.body = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>gRavioli</title></head><body>Hello World!</body></html>";
                } else {
                    res.status = 404;
                    res.reason = "Not Found";
                    res.version = _reqtype.version;
                    res.fields.insert("Content-Type", "text/html");
                    res.body = "The file was not found";
                }
            } catch (const std::exception& e) {
                Log::error("%s%", e.what());

                res.status = 500;
                res.reason = "Internal Error";
                res.version = _reqtype.version;
                res.fields.insert("Content-Type", "text/html");
                res.body = "An internal error occurred";
            }

            auto status = res.status;
            auto size = res.body.size();

            beast::http::prepare(res);
            async_write(_socket, std::move(res),
                        std::bind(&Peer::onWrite, shared_from_this(), beast::asio::placeholders::error));

            Log::acclog(ip, "", "", type, url, "HTTP", _reqtype.version, status, size);
        }

        void onWrite(const boost::system::error_code& e) {
            if (e) {
                Log::error("Failure during writing (error code: %d)", e);
                return;
            }

            read();
        }
    };

public:
    Server(const boost::asio::ip::tcp::endpoint& ep, std::size_t nthreads)
        : _acceptor(_ios)
        , _socket(_ios) {
        Log::info("Starting server...");

        _acceptor.open(ep.protocol());
        _acceptor.bind(ep);
        _acceptor.listen(boost::asio::socket_base::max_connections);

        auto onAccept_cb = std::bind(&Server::onAccept, this, beast::asio::placeholders::error);
        _acceptor.async_accept(_socket, onAccept_cb);

        _threads.reserve(nthreads);
        for (std::size_t i = 0; i < nthreads; i++) {
            _threads.emplace_back([&] { _ios.run(); });
        }

        Log::info("Server has started successfully.");
    }

    ~Server() {
        Log::info("Shutting down server...");
        boost::system::error_code e;
        _ios.dispatch([&] { _acceptor.close(e); });

        for (auto& t : _threads) {
            t.join();
        }
        Log::info("Server has stopped successfully.");
    }

    void onAccept(boost::system::error_code e) {
        if (!_acceptor.is_open() || e) {

            // this can hapen on shutdown
            Log::debug("Purgin %s.", __FUNCTION__);
            return;
        }

        // move _socket -> socket 
        auto socket(std::move(_socket));

        // renew socket on new acceptance
        auto onAccept_cb = std::bind(&Server::onAccept, this, beast::asio::placeholders::error);
        _acceptor.async_accept(_socket, onAccept_cb);

        std::make_shared<Peer>(std::move(socket), *this)->run();
    }
};

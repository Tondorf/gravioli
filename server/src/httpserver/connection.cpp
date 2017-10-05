#include <vector>

#include "httpserver/connection.hpp"
#include "httpserver/connectionManager.hpp"


namespace httpserver {
    Connection::Connection(boost::asio::ip::tcp::socket socket,
                           ConnectionManager& connectionManager,
                           IRequestHandler& requestHandler) :
        _socket(std::move(socket)),
        _connectionManager(connectionManager),
        _requestHandler(requestHandler) {
    }


    void Connection::start() {
        read();
    }


    void Connection::stop() {
        _socket.close();
    }


    void Connection::read() {
        auto self(shared_from_this());
        _socket.async_read_some(boost::asio::buffer(_buffer), [this, self](
            boost::system::error_code ec,
            std::size_t transferredBytes) {
            if (!ec) {
                RequestParser::ResultType result;
                std::tie(result, std::ignore) = _requestParser.parse(
                    _request,
                    _buffer.data(),
                    _buffer.data() + transferredBytes
                );

                if (result == RequestParser::ResultType::GOOD) {
                    _requestHandler.handleRequest(_request, _reply);
                    write();
                } else if (result == RequestParser::ResultType::BAD) {
                    _reply = Reply::stockReply(Reply::StatusType::BAD_REQUEST);
                    write();
                } else {
                    read();
                }
            } else if (ec != boost::asio::error::operation_aborted) {
                _connectionManager.stop(shared_from_this());
            }
        });
    }


    void Connection::write() {
        auto self(shared_from_this());
        boost::asio::async_write(_socket, _reply.toBuffers(), [this, self](
            boost::system::error_code ec,
            std::size_t) {
            if (!ec) {
                boost::system::error_code ignored;
                _socket.shutdown(
                    boost::asio::ip::tcp::socket::shutdown_both,
                    ignored
                );
            }

            if (ec != boost::asio::error::operation_aborted) {
                _connectionManager.stop(shared_from_this());
            }
        });
    }
}

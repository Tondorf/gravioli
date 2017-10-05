#include <string>

#include "httpserver/reply.hpp"


namespace httpserver {
    namespace statusStrings {
        const std::string OK = "200 OK";
        const std::string CREATED = "201 Created";
        const std::string ACCEPTED = "202 Accepted";
        const std::string NO_CONTENT = "204 No Content";
        const std::string MULTIPLE_CHOICES = "300 Multiple Choices";
        const std::string MOVED_PERMANENTLY = "301 Moved Permanently";
        const std::string MOVED_TEMPORARILY = "302 Moved Temporarily";
        const std::string NOT_MODIFIED = "304 Not Modified";
        const std::string BAD_REQUEST = "400 Bad Request";
        const std::string UNAUTHORIZED = "401 Unauthorized";
        const std::string FORBIDDEN = "403 Forbidden";
        const std::string NOT_FOUND = "404 Not Found";
        const std::string INTERNAL_SERVER_ERROR = "500 Internal Server Error";
        const std::string NOT_IMPLEMENTED = "501 Not Implemented";
        const std::string BAD_GATEWAY = "502 Bad Gateway";
        const std::string SERVICE_UNAVAILABLE = "503 Service Unavailable";

        std::string toString(Reply::StatusType status) {
            switch (status) {
                case Reply::StatusType::OK:
                    return OK;
                case Reply::StatusType::CREATED:
                    return CREATED;
                case Reply::StatusType::ACCEPTED:
                    return ACCEPTED;
                case Reply::StatusType::NO_CONTENT:
                    return NO_CONTENT;
                case Reply::StatusType::MULTIPLE_CHOICES:
                    return MULTIPLE_CHOICES;
                case Reply::StatusType::MOVED_PERMANENTLY:
                    return MOVED_PERMANENTLY;
                case Reply::StatusType::MOVED_TEMPORARILY:
                    return MOVED_TEMPORARILY;
                case Reply::StatusType::NOT_MODIFIED:
                    return NOT_MODIFIED;
                case Reply::StatusType::BAD_REQUEST:
                    return BAD_REQUEST;
                case Reply::StatusType::UNAUTHORIZED:
                    return UNAUTHORIZED;
                case Reply::StatusType::FORBIDDEN:
                    return FORBIDDEN;
                case Reply::StatusType::NOT_FOUND:
                    return NOT_FOUND;
                case Reply::StatusType::INTERNAL_SERVER_ERROR:
                    return INTERNAL_SERVER_ERROR;
                case Reply::StatusType::NOT_IMPLEMENTED:
                    return NOT_IMPLEMENTED;
                case Reply::StatusType::BAD_GATEWAY:
                    return BAD_GATEWAY;
                case Reply::StatusType::SERVICE_UNAVAILABLE:
                    return SERVICE_UNAVAILABLE;
                default:
                    return INTERNAL_SERVER_ERROR;
            }
        }
    }


    std::vector<boost::asio::const_buffer> Reply::toBuffers() {
        std::vector<boost::asio::const_buffer> buffers;
        /*
        auto add = [&buffers](const std::string& line,
                              const std::string& suffix = "\r\n") {
            buffers.push_back(boost::asio::buffer(line + suffix));
        };

        add(std::string("HTTP/1.0 ") + statusStrings::toString(status));
        for (std::size_t i = 0; i < headers.size(); ++i) {
            auto& h = headers[i];
            add(h.name + ": " + h.value);
        }
        add("");
        add(content, "");
        */
        std::string foo = "foo";
        buffers.push_back(boost::asio::buffer(foo, foo.size()));

        return buffers;
    }


    Reply Reply::stockReply(StatusType status) {
        Reply reply;
        reply.status = status;

        auto format = [](const std::string& s) { 
            return std::string("<html><head><title>")
                   + s + "</title></head><body><h1>"
                   + s + "</h1></body></html>";
        };
        reply.content = format(statusStrings::toString(status));
        reply.headers.resize(2);
        reply.headers[0].name = "Content-Length";
        reply.headers[0].value = std::to_string(reply.content.size());
        reply.headers[1].name = "Content-Type";
        reply.headers[1].value = "text/html";

        return reply;
    }
}

#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "httpserver/header.hpp"


namespace httpserver {
    struct Reply {
        enum class StatusType {
            OK = 200,
            CREATED = 201,
            ACCEPTED = 202,
            NO_CONTENT = 204,
            MULTIPLE_CHOICES = 300,
            MOVED_PERMANENTLY = 301,
            MOVED_TEMPORARILY = 302,
            NOT_MODIFIED = 304,
            BAD_REQUEST = 400,
            UNAUTHORIZED = 401,
            FORBIDDEN = 403,
            NOT_FOUND = 404,
            INTERNAL_SERVER_ERROR = 500,
            NOT_IMPLEMENTED = 501,
            BAD_GATEWAY = 502,
            SERVICE_UNAVAILABLE = 503
        } status;

        std::vector<Header> headers;

        std::string content;

        std::vector<boost::asio::const_buffer> toBuffers();

        static Reply stockReply(Reply::StatusType);
    };
}

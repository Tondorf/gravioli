#pragma once

#include "httpserver/requestHandler.hpp"


namespace authentication {
    class RequestHandler : public httpserver::IRequestHandler {
    public:
        RequestHandler() = default;

        virtual ~RequestHandler() = default;

        RequestHandler(const RequestHandler&) = delete;

        RequestHandler& operator=(const RequestHandler&) = delete;

        void handleRequest(const httpserver::Request&,
                           httpserver::Reply&);
    };
}

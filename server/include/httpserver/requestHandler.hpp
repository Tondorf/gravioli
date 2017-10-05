#pragma once

#include <string>

#include "httpserver/request.hpp"
#include "httpserver/reply.hpp"


namespace httpserver {
    class IRequestHandler {
    protected:
        static std::string formatLog(const Request& req, const Reply& rep) {
            return std::string("\"") + req.method + " " + req.uri + " HTTP/" +
                   std::to_string(req.http_version_major) + "." + 
                   std::to_string(req.http_version_minor) + "\" " + 
                   std::to_string(static_cast<int>(rep.status)) + " " +
                   std::to_string(rep.content.size());
        }

    public:
        virtual void handleRequest(const Request&, Reply&) = 0;
    };
}

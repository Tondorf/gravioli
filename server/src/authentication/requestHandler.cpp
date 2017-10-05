#include "authentication/requestHandler.hpp"

#include "SimpleLogger/logger.hpp"


namespace authentication {
    void RequestHandler::handleRequest(const httpserver::Request& req,
                                       httpserver::Reply& rep) {
        using namespace httpserver;
        rep = Reply::stockReply(Reply::StatusType::NOT_FOUND);

        Log::info(formatLog(req, rep).c_str());
    }
}

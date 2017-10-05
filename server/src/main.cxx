#include <cstdlib>
#include <future>

#include <boost/asio.hpp>

#include "SimpleLogger/logger.hpp"

#include "config.hpp"

#include "common/userInputParser.hpp"
#include "common/greenwich.hpp"
#include "server/server.hpp"
#include "server/simpleMsgQueue.hpp"
#include "httpserver/server.hpp"
#include "authentication/requestHandler.hpp"


void awaitShutdown(boost::asio::signal_set &signal,
                   std::function<void(boost::system::error_code, int)> cb) {
    signal.add(SIGINT);
    signal.add(SIGTERM);
#if defined SIGQUIT
    signal.add(SIGQUIT);
#endif
    signal.async_wait(cb);
}


int main(int argc, char const* argv[]) {
    auto logLevel = []() {
        if (BUILD_TYPE == BuildType::RELEASE) {
            return Log::LogLevel::INFO;
        }
        return Log::LogLevel::DEBUG;
    }();
    Log::SimpleLogger::getInstance().setLogLevel(logLevel);

    /*
     * Greenwich is initialize on first call of Greenwich::getInstance().
     * Ensure, that following call is this first call!
     */
    Greenwich::getInstance();

    Log::SimpleLogger::getInstance().addTimestamp([]() {
        return Greenwich::getInstance().getFormatedTime() + " ";
    });

    Config config;
    if (!parseUserInput(argc, argv, config)) {
        return EXIT_FAILURE;
    }

    Log::info("Log level: %s", []() {
        auto logLevel = Log::SimpleLogger::getInstance().getLogLevel();
        return Log::SimpleLogger::logLevelAsString(logLevel);
    }().c_str());

    Log::info("Version: %d.%d (%s)",
              VERSION_MAJOR, VERSION_MINOR, BUILD_TYPE_AS_STRING.c_str());

    using MsgQueue = server::SimpleMsgQueue;
    server::Server server(config.port, std::make_shared<MsgQueue>());

    using RequestHandler = authentication::RequestHandler;
    httpserver::Server<RequestHandler> authSrv("127.0.0.1", 8080);

    boost::asio::io_service ios;
    boost::asio::signal_set shutdownSignal(ios);
    awaitShutdown(shutdownSignal, [&server, &authSrv](boost::system::error_code,
                                            int /*signo*/) {
        Log::info("Captured SIGINT, SIGTERM or SIGQUIT.");
        server.stop();
        authSrv.stop();
    });

    auto serverExitedSuccessfully = std::async(
        std::launch::async, [&server]() {
            return server.run();
        }
    );

    auto authSrvExitedSuccessfully = std::async(
        std::launch::async, [&authSrv]() {
            return authSrv.run();
        }
    );

    /*
     * this blocks until shutdown is triggered
     */
    ios.run();

    if (serverExitedSuccessfully.get()) {
        Log::info("Stopped server successfully.");
    } else {
        Log::error("Server exited with errors!");
    }

    if (authSrvExitedSuccessfully.get()) {
        Log::info("Stopped authentication service successfully.");
    } else {
        Log::error("Authentication service exited with errors!");
    }

    return EXIT_SUCCESS;
}

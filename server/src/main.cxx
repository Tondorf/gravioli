#include <cstdlib>
#include <thread>

#include <boost/asio.hpp>

#include "SimpleLogger/logger.hpp"

#include "config.hpp"
#include "userInputParser.hpp"

#include "server/greenwich.hpp"
#include "server/server.hpp"


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

    Log::info("Version: %d.%d (%s)", VERSION_MAJOR, VERSION_MINOR, BUILD_TYPE_AS_STRING.c_str());
    Log::info("Starting %d thread(s), bound to port %d.", config.threads, config.port);

    server::Server server(config.port, config.threads);

    boost::asio::io_service ios;
    boost::asio::signal_set shutdownSignal(ios);
    awaitShutdown(shutdownSignal, [&server](boost::system::error_code /*ec*/, int /*signo*/) {
        Log::info("Captured SIGINT, SIGTERM or SIGQUIT.");
        server.stop();
    });
    std::thread shutdownScout([&ios]() { ios.run(); });

    /*
     * Server::run blocks as long as server is running.
     * Server is stopped by calling Server::stop.
     */
    if (server.run()) {
        Log::info("Stopped server successfully.");
    } else {
        Log::error("Server exited with errors!");
    }

    shutdownScout.join();

    return EXIT_SUCCESS;
}

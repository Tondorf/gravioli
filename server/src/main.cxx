#include <cstdlib>
#include <future>

#include <boost/asio.hpp>

#include "SimpleLogger/logger.hpp"

#include "config.hpp"

#include "userInputParser.hpp"
#include "greenwich.hpp"
#include "server.hpp"
#include "simpleMsgQueue.hpp"
#include "server.hpp"
#include "serializableWorld.hpp"


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
              VERSION_MAJOR, VERSION_MINOR, BUILD_TYPE_AS_STRING);

    auto msgQueue = std::make_shared<server::SimpleMsgQueue>();
    server::Server server(config.port, msgQueue);

    simulation::WorldProperties wprop;
    std::vector<std::shared_ptr<simulation::SerializableWorld>> worlds;
    simulation::SerializableWorld::init(worlds, wprop, msgQueue);

    boost::asio::io_service ios;
    boost::asio::signal_set shutdownSignal(ios);
    awaitShutdown(shutdownSignal, [&server, &worlds](boost::system::error_code,
                                                     int /*signo*/) {
        Log::info("Captured SIGINT, SIGTERM or SIGQUIT.");
        for (auto&& w : worlds) {
            w->stop();
        }
        server.stop();
    });

    auto serverExitedSuccessfully = std::async(
        std::launch::async, [&server]() {
            return server.run();
        }
    );

    std::vector<std::future<bool>> worldsExitStatus;
    for (auto&& world : worlds) {
        worldsExitStatus.push_back(std::async(
            std::launch::async, [world]() {
                return world->run();
            }
        ));
    }

    /*
     * this blocks until shutdown is triggered
     */
    ios.run();

    bool allWorldsStopsSuccessfully = true;
    for (auto&& worldExitedSuccessfully : worldsExitStatus) {
        if (!worldExitedSuccessfully.get()) {
            allWorldsStopsSuccessfully = false;
        }
    }
    if (allWorldsStopsSuccessfully) {
        Log::info("Stopped all worlds successfully.");
    } else {
        Log::error("Some of the worlds exited with errors!");
    }

    if (serverExitedSuccessfully.get()) {
        Log::info("Stopped server successfully.");
    } else {
        Log::error("Server exited with errors!");
    }

    return EXIT_SUCCESS;
}

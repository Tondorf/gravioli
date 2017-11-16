#include <boost/asio.hpp>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <future>
#include <iomanip>
#include <memory>
#include <new>
#include <ostream>
#include <string>
#include <vector>
#include "SimpleLogger/logger.hpp"
#include "config.hpp"
#include "serializableWorld.hpp"
#include "server.hpp"
#include "simpleMsgQueue.hpp"
#include "userInputParser.hpp"


void configureLogger();
void awaitShutdown(boost::asio::signal_set &,
                   std::function<void(boost::system::error_code, int)>);


int main(int argc, char const* argv[]) {
    Config config;
    if (!parseUserInput(argc, argv, config)) {
        return EXIT_FAILURE;
    }

    configureLogger();

    Log::info("Version: %d.%d (%s)",
              VERSION_MAJOR, VERSION_MINOR, BUILD_TYPE_AS_STRING);

    auto msgQueue = std::make_shared<server::SimpleMsgQueue>();
    server::Server server(config.port, msgQueue);

    std::vector<std::shared_ptr<simulation::SerializableWorld>> worlds;
    simulation::SerializableWorld::init(worlds, msgQueue);

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


void configureLogger() {
    auto logLevel = []() {
        if (BUILD_TYPE == BuildType::RELEASE) {
            return Log::LogLevel::INFO;
        }
        return Log::LogLevel::DEBUG;
    }();
    Log::SimpleLogger::getInstance().setLogLevel(logLevel);

    std::locale::global(std::locale(LOGDATE_LOCALE));
    Log::SimpleLogger::getInstance().addTimestamp([]() {
        auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), LOGDATE_FORMAT);

        return ss.str() + " ";
    });

    Log::info("Log level: %s", []() {
        auto logLevel = Log::SimpleLogger::getInstance().getLogLevel();
        return Log::SimpleLogger::logLevelAsString(logLevel);
    }().c_str());
}


void awaitShutdown(boost::asio::signal_set &signal,
                   std::function<void(boost::system::error_code, int)> cb) {
    signal.add(SIGINT);
    signal.add(SIGTERM);
#if defined SIGQUIT
    signal.add(SIGQUIT);
#endif
    signal.async_wait(cb);
}

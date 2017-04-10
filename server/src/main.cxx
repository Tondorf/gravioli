#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "config.h"
#include "logger.hpp"

#include "server.hpp"


int main(int argc, char** argv) {
    namespace po = boost::program_options;

    po::options_description desc("Options");
    desc.add_options()
        ("help,h",  "Show available options")
        ("port,p",  po::value<std::uint16_t>()->default_value(DEFAULT_PORT),
                    "Set the port number for the server")
        ("threads,n", po::value<std::size_t>()->default_value(DEFAULT_NTHREADS),
                      "Set the number of threads to use");
    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    } catch(const boost::program_options::error &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    std::ifstream splashfile(SPLASH_FILE);
    if (!splashfile.is_open()) {
        Log::error("Could not print splash since file \"%s\" was not found.", SPLASH_FILE);
    } else {
        std::string line;
        while (getline(splashfile, line)) {
            std::cout << line << std::endl;
        }
        splashfile.close();
    }
    std::cout << "(version: " << VERSION_MAJOR << "." << VERSION_MINOR << ")" << std::endl;
    std::cout << std::endl;

    auto logLevel = Log::LogLevel::DEBUG;
    Log::SimpleLogger::getInstance().setLogLevel(logLevel);
    auto logLevelAsString = Log::SimpleLogger::getInstance().getLogLevelAsString();
    Log::info("Log level: %s.", logLevelAsString.c_str());

    auto port = vm["port"].as<std::uint16_t>();
    auto nthreads = vm["threads"].as<std::size_t>();
    Log::info("Starting %d threads, listing on port %d.", nthreads, port);

    auto addr = boost::asio::ip::address::from_string(DEFAULT_ADDRESS);
    boost::asio::ip::tcp::endpoint ep(addr, port);

    Server server(ep, nthreads);

	boost::asio::io_service ios;
    boost::asio::signal_set signals(ios, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code const&, int) {});
    ios.run();

    return EXIT_SUCCESS;
}

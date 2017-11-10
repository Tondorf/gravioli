#include <cassert>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include "SimpleLogger/logger.hpp"
#include "userInputParser.hpp"


bool parseUserInput(int argc, char const* argv[], Config& config) {
    namespace po = boost::program_options;
    
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help",    "Print help message")
        ("version", "Print version")
        ("port",    po::value<port_t>()->default_value(DEFAULT_PORT),
                    "Set the port number for the server");

    bool parsingSuccessfully = true;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch(po::error& e) {
        parsingSuccessfully = false;
        Log::error("Couldn't parse user input; %s", e.what());
    }
    
    if (!parsingSuccessfully || vm.count("help")) {
        std::cout << desc << std::endl;
        return false;
    }

    if (vm.count("version")) {
        std::cout << "Vesion " << VERSION_MAJOR << "." << VERSION_MINOR;
        std::cout << " (" << BUILD_TYPE_AS_STRING << ")" << std::endl;
        return false;
    }

    assert(vm.count("port"));

    config.port = vm["port"].as<port_t>();

    return true;
}

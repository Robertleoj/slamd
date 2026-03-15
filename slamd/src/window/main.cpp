#include <flatb/visualizer_generated.h>
#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <cstdlib>
#include <cxxopts.hpp>
#include <iostream>
#include <slamd_window/run_window.hpp>

int main(
    int argc,
    char* argv[]
) {
    spdlog::set_level(spdlog::level::debug);

    cxxopts::Options options(
        "slamd Window",
        "The viewer for the slamd library"
    );

    // clang-format off
    options
        .add_options()
        (
            "p,port", 
            "Port number to use", 
            cxxopts::value<int>()->default_value("5555")
        )
        (
            "i,ip", 
            "IP address to connect to", 
            cxxopts::value<std::string>()->default_value("127.0.0.1")
        )
        (
            "h,help",
            "Show help"
        );
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    uint16_t port = result["port"].as<int>();
    std::string ip = result["ip"].as<std::string>();

    slamd::StateManager state_manager;

    state_manager.try_connect(ip, port);

    slamd::run_window(state_manager);

    std::exit(0);
}
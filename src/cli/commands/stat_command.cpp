#include "stat_command.h"
#include "../../interfaces/i_statistic_calculator.h" // <-- Include the interface, NOT the concrete class
#include <cxxopts.hpp>
#include <iostream>

namespace fq::app {

auto StatCommand::execute(int argc, char* argv[]) -> int {
    cxxopts::Options options(getName(), getDescription());
    options.add_options()
        ("i,input", "Input FASTQ file", cxxopts::value<std::string>())
        ("o,output", "Output statistics file", cxxopts::value<std::string>())
        ("t,threads", "Number of threads", cxxopts::value<int>()->default_value("1"))
        ("h,help", "Print usage");

    if (argc == 1) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // Use the interface-level options struct
    fq::statistic::StatisticOptions stat_options;
    stat_options.input_fastq = result["input"].as<std::string>();
    stat_options.output_stat = result["output"].as<std::string>();
    stat_options.thread_num = result["threads"].as<int>();

    // Use the factory to create an instance of the calculator
    auto stater = fq::statistic::create_statistic_calculator(stat_options);
    
    // Call run via the interface pointer
    stater->run();

    return 0;
}

auto StatCommand::getName() const -> std::string {
    return "stat";
}

auto StatCommand::getDescription() const -> std::string {
    return "Generate statistics for a FASTQ file";
}

} // namespace fq::app

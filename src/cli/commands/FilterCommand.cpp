#include "cli/commands/FilterCommand.h"

#include <iomanip>
#include <iostream>

#include "Processing/Mutators/QualityTrimmer.h"
#include "Processing/Predicates/MinQualityPredicate.h"
#include "Processing/ProcessingPipeline.h"  // Add full definition for ProcessingStatistics
#include "cxxopts.hpp"
#include "interfaces/IProcessingPipeline.h"  // <-- Include the interface

namespace fq::app {

struct FilterCommand::Config {
    std::string input_file;
    std::string output_file;
    size_t thread_count = 1;
};

// Use the factory in the constructor
FilterCommand::FilterCommand()
    : m_config(std::make_unique<Config>()), m_pipeline(fq::processing::create_processing_pipeline()) {}

FilterCommand::~FilterCommand() = default;

auto FilterCommand::execute(int argc, char* argv[]) -> int {
    cxxopts::Options options(getName(), getDescription());
    options.add_options()("i,input", "Input FASTQ file", cxxopts::value<std::string>())("o,output", "Output FASTQ file",
                                                                                        cxxopts::value<std::string>())(
        "t,threads", "Number of threads", cxxopts::value<size_t>()->default_value("1"))("h,help", "Print usage");

    if (argc == 1) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    m_config->input_file = result["input"].as<std::string>();
    m_config->output_file = result["output"].as<std::string>();
    m_config->thread_count = result["threads"].as<size_t>();

    m_pipeline->setInput(m_config->input_file);
    m_pipeline->setOutput(m_config->output_file);

    // Use the config from the interface
    fq::processing::ProcessingConfig pipeline_config;
    pipeline_config.thread_count = m_config->thread_count;
    m_pipeline->setConfig(pipeline_config);

    m_pipeline->run();

    return 0;
}

auto FilterCommand::getName() const -> std::string {
    return "filter";
}

auto FilterCommand::getDescription() const -> std::string {
    return "Filter and trim FastQ files";
}

}  // namespace fq::app
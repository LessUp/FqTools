#include "filter_command.h"

#include <iomanip>
#include <iostream>

#include "pipeline/processing/mutators/quality_trimmer.h"
#include "pipeline/processing/predicates/min_quality_predicate.h"
#include "pipeline/processing/processing_pipeline.h"  // Add full definition for ProcessingStatistics
#include <cxxopts.hpp>
#include <fqtools/processing_pipeline.h>  // public API for interface

namespace fq::app {

struct FilterCommand::Config {
    std::string input_file;
    std::string output_file;
    size_t thread_count = 1;
};

// Use the factory in the constructor
FilterCommand::FilterCommand()
    : m_config(std::make_unique<Config>()), m_pipeline(fq::processing::make_processing_pipeline()) {}

FilterCommand::~FilterCommand() = default;

auto FilterCommand::execute(int argc, char* argv[]) -> int {
    cxxopts::Options options(getName(), getDescription());
    options.add_options()
        ("i,input", "Input FASTQ file", cxxopts::value<std::string>())
        ("o,output", "Output FASTQ file", cxxopts::value<std::string>())
        ("t,threads", "Number of threads", cxxopts::value<size_t>()->default_value("1"))
        ("quality-encoding", "Quality encoding offset (33 or 64)", cxxopts::value<int>()->default_value("33"))
        ("min-quality", "Minimum average quality threshold", cxxopts::value<double>())
        ("min-length", "Minimum read length", cxxopts::value<size_t>())
        ("max-length", "Maximum read length", cxxopts::value<size_t>())
        ("max-n-ratio", "Maximum N ratio (0.0-1.0)", cxxopts::value<double>())
        ("trim-quality", "Trim bases below quality threshold", cxxopts::value<double>())
        ("trim-mode", "Trim mode (both,five,three)", cxxopts::value<std::string>()->default_value("both"))
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

    m_config->input_file = result["input"].as<std::string>();
    m_config->output_file = result["output"].as<std::string>();
    m_config->thread_count = result["threads"].as<size_t>();

    m_pipeline->setInput(m_config->input_file);
    m_pipeline->setOutput(m_config->output_file);

    // Use the config from the interface
    fq::processing::ProcessingConfig pipeline_config;
    pipeline_config.thread_count = m_config->thread_count;
    m_pipeline->setConfig(pipeline_config);

    // Wire predicates and mutators from CLI options
    const int quality_encoding = result["quality-encoding"].as<int>();

    if (result.count("min-quality")) {
        double min_q = result["min-quality"].as<double>();
        m_pipeline->addPredicate(std::make_unique<fq::processing::MinQualityPredicate>(min_q, quality_encoding));
    }

    if (result.count("min-length")) {
        size_t min_len = result["min-length"].as<size_t>();
        m_pipeline->addPredicate(std::make_unique<fq::processing::MinLengthPredicate>(min_len));
    }

    if (result.count("max-length")) {
        size_t max_len = result["max-length"].as<size_t>();
        m_pipeline->addPredicate(std::make_unique<fq::processing::MaxLengthPredicate>(max_len));
    }

    if (result.count("max-n-ratio")) {
        double max_n = result["max-n-ratio"].as<double>();
        m_pipeline->addPredicate(std::make_unique<fq::processing::MaxNRatioPredicate>(max_n));
    }

    if (result.count("trim-quality")) {
        double trim_q = result["trim-quality"].as<double>();
        std::string mode_str = result["trim-mode"].as<std::string>();
        fq::processing::QualityTrimmer::TrimMode mode = fq::processing::QualityTrimmer::TrimMode::Both;
        if (mode_str == "five") mode = fq::processing::QualityTrimmer::TrimMode::FivePrime;
        else if (mode_str == "three") mode = fq::processing::QualityTrimmer::TrimMode::ThreePrime;
        m_pipeline->addMutator(std::make_unique<fq::processing::QualityTrimmer>(trim_q, /*min_length*/1, mode, quality_encoding));
    }

    auto stats = m_pipeline->run();
    std::cout << stats.toString() << std::endl;

    return 0;
}

auto FilterCommand::getName() const -> std::string {
    return "filter";
}

auto FilterCommand::getDescription() const -> std::string {
    return "Filter and trim FastQ files";
}

}  // namespace fq::app
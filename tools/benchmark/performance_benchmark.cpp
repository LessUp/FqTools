#include <iostream>
#include "cxxopts.hpp"
#include "common/Timer.h"
#include "common/UI.h"
#include "cli/commands/StatCommand.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/**
 * @brief A dedicated benchmark runner for fastqtools.
 * 
 * This program is designed to execute specific, performance-critical
 * scenarios to establish a baseline and measure improvements from
 * refactoring efforts.
 */
int main(int argc, char* argv[]) {
    fq::common::print_big_logo();
    fq::common::software_info("FastQTools Benchmark");

    cxxopts::Options options("performance_benchmark", "Run performance benchmarks for fastqtools.");
    options.add_options()
        ("i,input", "Input FASTQ file for the benchmark", cxxopts::value<std::string>())
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help") || !result.count("input")) {
        std::cout << options.help() << std::endl;
        return 1;
    }

    // Setup default logger
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn); // Keep output clean, only show warnings/errors
    console_sink->set_pattern("[%^%l%$] %v");
    auto logger = std::make_shared<spdlog::logger>("benchmark", console_sink);
    spdlog::set_default_logger(logger);

    std::string input_file = result["input"].as<std::string>();

    spdlog::info("Starting benchmark for input file: {}", input_file);

    fq::common::Timer benchmark_timer("Total Benchmark Run");

    try {
        // --- Benchmark Scenario: `stat` command ---
        // We simulate running `fastqtools stat -i <input_file>`
        fq::cli::StatCommand stat_command;
        
        // Create a fake ParseResult for the command
        // In a real scenario, cxxopts would create this. We need to manually construct it.
        // This is a simplified way to call the command's logic directly.
        // NOTE: This is a bit of a hack. A better long-term solution might be to
        // refactor command logic to be independent of cxxopts::ParseResult.
        // For now, we create a minimal command line to parse.
        int fake_argc = 3;
        const char* fake_argv[] = {"stat", "-i", input_file.c_str()};
        
        cxxopts::Options stat_options("stat", "");
        stat_command.setup_options(stat_options);
        auto stat_result = stat_options.parse(fake_argc, fake_argv);

        spdlog::info("Executing 'stat' command scenario...");
        fq::common::Timer stat_timer("Stat Command");
        
        int exit_code = stat_command.execute(stat_result);

        stat_timer.report(false);

        if (exit_code != 0) {
            spdlog::error("Benchmark scenario 'stat' failed with exit code {}", exit_code);
            return 1;
        }

    } catch (const std::exception& e) {
        spdlog::error("An exception occurred during benchmark: {}", e.what());
        return 1;
    }

    benchmark_timer.report(false);
    
    // Here we could also add system calls to report peak memory usage, etc.
    // For now, we rely on the timer output.

    return 0;
}

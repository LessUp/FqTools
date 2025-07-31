#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Core/Core.h"
#include "cli/commands/ICommand.h"
#include "cli/commands/StatCommand.h"
#include "cli/commands/FilterCommand.h"
#include "spdlog/spdlog.h"

namespace fq::cli {
    // Forward declare to avoid exposing cxxopts in headers
    void print_global_help(const std::map<std::string, CommandPtr>& commands);
}

auto main(int argc, char* argv[]) -> int {
    fq::common::print_big_logo();
    fq::common::software_info("FastQTools");
    fq::common::Timer main_timer("FastQTools");
    spdlog::set_level(spdlog::level::info);

    std::map<std::string, fq::cli::CommandPtr> commands;
    commands["stat"] = std::make_unique<fq::app::StatCommand>();
    commands["filter"] = std::make_unique<fq::app::FilterCommand>();

    if (argc < 2) {
        fq::cli::print_global_help(commands);
        return 1;
    }

    std::string subcommand = argv[1];
    auto command_iterator = commands.find(subcommand);
    if (command_iterator == commands.end()) {
        spdlog::error("Unknown subcommand: {}", subcommand);
        fq::cli::print_global_help(commands);
        return 1;
    }

    try {
        // Create a new argc and argv for the subcommand
        int sub_argc = argc - 1;
        char** sub_argv = argv + 1;
        return command_iterator->second->execute(sub_argc, sub_argv);
    } catch (const std::exception& e) {
        spdlog::error("An error occurred: {}", e.what());
        return 1;
    }
}

namespace fq::cli {
void print_global_help(const std::map<std::string, CommandPtr>& commands) {
    std::cout << "Usage: fastqtools <command> [options]\n\n";
    std::cout << "Available commands:\n";
    for (const auto& [name, command] : commands) {
        std::cout << "  " << name << "\t\t" << command->getDescription() << "\n";
    }
    std::cout << "\nRun 'fastqtools <command> --help' for more information on a specific command.\n";
}
}
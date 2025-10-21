/**
 * @file main.cpp
 * @brief FastQTools 命令行主程序入口
 * @details 负责解析命令行参数，分发子命令，并输出帮助信息。
 * @author FastQTools Team
 * @date 2025-08-01
 * @version 1.0
 * @copyright Copyright (c) 2025 FastQTools
 */

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <fqtools/fq.h>
#include "commands/i_command.h"
#include "commands/stat_command.h"
#include "commands/filter_command.h"
#include "spdlog/spdlog.h"

namespace fq::cli {
    // Forward declare to avoid exposing cxxopts in headers
    void print_global_help(const std::map<std::string, CommandPtr>& commands);
}

/**
 * @brief FastQTools 主函数，命令行程序入口
 * @details 负责初始化日志、计时器，解析参数并分发到具体子命令。
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序执行状态码，0 表示成功，非0表示异常
 */
auto main(int argc, char* argv[]) -> int {
    // 打印项目 Logo
    fq::common::print_big_logo();
    // 输出软件信息
    fq::common::software_info("FastQTools");
    // 启动主计时器
    fq::common::Timer main_timer("FastQTools");
    // 设置日志级别为 info
    spdlog::set_level(spdlog::level::info);

    // 注册支持的子命令
    std::map<std::string, fq::cli::CommandPtr> commands;
    commands["stat"] = std::make_unique<fq::app::StatCommand>();
    commands["filter"] = std::make_unique<fq::app::FilterCommand>();

    // 检查参数数量，若无子命令则输出帮助
    if (argc < 2) {
        fq::cli::print_global_help(commands);
        return 1;
    }

    // 获取子命令名称
    std::string subcommand = argv[1];
    // 查找对应子命令
    auto command_iterator = commands.find(subcommand);
    if (command_iterator == commands.end()) {
        spdlog::error("Unknown subcommand: {}", subcommand);
        fq::cli::print_global_help(commands);
        return 1;
    }

    try {
        // 捕获并处理子命令执行过程中的异常
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
/**
 * @brief 输出全局帮助信息
 * @details 打印所有可用子命令及其描述。
 * @param commands 命令名称与命令对象的映射表
 */
void print_global_help(const std::map<std::string, CommandPtr>& commands) {
    std::cout << "Usage: FastQTools <command> [options]\n\n";
    std::cout << "Available commands:\n";
    for (const auto& [name, command] : commands) {
        std::cout << "  " << name << "\t\t" << command->getDescription() << "\n";
    }
    std::cout << "\nRun 'FastQTools <command> --help' for more information on a specific command.\n";
}
}
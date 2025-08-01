/**
 * @file StatCommand.h
 * @brief 定义了 'stat' 子命令。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#pragma once
#include "commands/i_command.h"

namespace fq::app {

/**
 * @brief 实现了用于生成 FastQ 文件统计信息的 'stat' 命令。
 */
class StatCommand : public fq::cli::ICommand {
public:
    auto execute(int argc, char* argv[]) -> int override;
    auto getName() const -> std::string override;
    auto getDescription() const -> std::string override;
};

}

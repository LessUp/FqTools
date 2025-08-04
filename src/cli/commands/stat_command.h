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
    /**
     * @brief 执行统计命令
     * @details 解析命令行参数并执行 FastQ 文件统计信息生成
     * 
     * @param argc 参数数量
     * @param argv 参数数组
     * @return 执行成功返回0，失败返回非0值
     */
    auto execute(int argc, char* argv[]) -> int override;

    /**
     * @brief 获取命令名称
     * @return 返回命令名称字符串 "stat"
     */
    auto getName() const -> std::string override;

    /**
     * @brief 获取命令描述
     * @return 返回命令功能的描述字符串
     */
    auto getDescription() const -> std::string override;
};

}

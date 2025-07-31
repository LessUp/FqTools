/**
 * @file FilterCommand.h
 * @brief 定义了 'filter' 子命令。
 * @author BGI-Research
 * @version 1.0
 * @date 2025-07-31
 * @copyright Copyright (c) 2025 BGI-Research
 */

#pragma once

#include "cli/commands/ICommand.h"
#include <memory>
#include <vector>
#include <string>

namespace fq::processing {
    // Forward declare the INTERFACE, not the concrete class
    class IProcessingPipeline;
}

namespace fq::app {

/**
 * @brief 实现了用于过滤和修剪 FastQ 文件的 'filter' 命令。
 */
class FilterCommand : public fq::cli::ICommand {
public:
    FilterCommand();
    ~FilterCommand();
    auto execute(int argc, char* argv[]) -> int override;
    auto getName() const -> std::string override;
    auto getDescription() const -> std::string override;

private:
    struct Config;
    std::unique_ptr<Config> m_config;
    // Depend on the INTERFACE
    std::unique_ptr<fq::processing::IProcessingPipeline> m_pipeline;
};

} // namespace fq::app

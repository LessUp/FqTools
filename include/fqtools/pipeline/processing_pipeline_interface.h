#pragma once

/**
 * @file i_processingPipeline.h
 * @brief 处理管道接口定义
 * @details 该模块定义了 FastQ 数据处理管道的抽象接口，
 *          实现了命令行层与具体实现之间的解耦
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "fqtools/pipeline/read_mutator_interface.h"
#include "fqtools/pipeline/read_predicate_interface.h"

namespace fq::processing
{

// 统计信息和处理器接口的前向声明
struct ProcessingStatistics;
using IReadMutator = ReadMutatorInterface;
using IReadPredicate = ReadPredicateInterface;

/**
 * @brief 处理管道配置结构体
 * @details 定义处理管道的各项配置参数，在接口级别定义以实现客户端解耦
 *          该配置包含了批处理大小和线程数等关键性能参数
 * 
 * @note 所有参数都有合理的默认值
 */
struct ProcessingConfig
{
    size_t batch_size = 10000;    ///< 批处理大小（每个批次的读取数量）
    size_t thread_count = 1;      ///< 线程数量（1表示串行处理）
};

/**
 * @brief 处理管道抽象接口
 * @details 定义了 FastQ 数据处理管道的核心接口，实现了命令行层与具体实现之间的解耦
 *          该接口提供了完整的处理流程控制，包括输入输出设置、配置管理、
 *          处理器注册和执行等功能
 * 
 * @invariant 所有配置必须在调用 run() 之前完成
 * @note 该接口支持多种实现方式，如串行处理、并行处理等
 */
class ProcessingPipelineInterface
{
public:
    /**
     * @brief 虚析构函数
     * @details 确保派生类的正确析构
     */
    virtual ~ProcessingPipelineInterface() = default;

    /**
     * @brief 设置输入文件路径
     * @details 指定要处理的 FastQ 文件路径
     * 
     * @param input_path 输入文件路径
     * @pre input_path 必须是有效的文件路径
     * @post 输入路径被设置
     * @throw std::invalid_argument 如果路径无效
     */
    virtual void setInput(const std::string &input_path) = 0;
    
    /**
     * @brief 设置输出文件路径
     * @details 指定处理结果的输出文件路径
     * 
     * @param output_path 输出文件路径
     * @pre output_path 必须是有效的文件路径
     * @post 输出路径被设置
     * @throw std::invalid_argument 如果路径无效
     */
    virtual void setOutput(const std::string &output_path) = 0;
    
    /**
     * @brief 设置处理配置
     * @details 配置处理参数，包括批处理大小和线程数等
     * 
     * @param config 处理配置结构体
     * @pre config 必须包含有效的配置参数
     * @post 配置被应用到处理管道
     * @throw std::invalid_argument 如果配置参数无效
     */
    virtual void setConfig(const ProcessingConfig &config) = 0;
    
    /**
     * @brief 添加数据修改器
     * @details 注册一个数据修改器，用于对读取数据进行转换处理
     * 
     * @param mutator 数据修改器的唯一指针
     * @pre mutator 必须是有效的修改器实例
     * @post 修改器被注册到处理管道中
     * @note 修改器按添加顺序依次执行
     */
    virtual void addMutator(std::unique_ptr<ReadMutatorInterface> mutator) = 0;
    
    /**
     * @brief 添加数据过滤器
     * @details 注册一个数据过滤器，用于筛选符合条件的读取
     * 
     * @param predicate 数据过滤器的唯一指针
     * @pre predicate 必须是有效的过滤器实例
     * @post 过滤器被注册到处理管道中
     * @note 过滤器按添加顺序依次执行，任一过滤器失败则数据被过滤
     */
    virtual void addPredicate(std::unique_ptr<ReadPredicateInterface> predicate) = 0;

    /**
     * @brief 执行数据处理
     * @details 启动完整的 FastQ 数据处理流程
     * 
     * @return ProcessingStatistics 处理统计信息
     * @pre 输入路径、输出路径和配置必须已设置
     * @post 处理结果写入输出文件
     * @throw std::runtime_error 如果处理过程中发生错误
     */
    virtual auto run() -> ProcessingStatistics = 0;
};

/**
 * @brief 创建处理管道实例的工厂函数
 * @details 创建并返回一个实现了 IProcessingPipeline 接口的对象实例
 *          该函数实现了工厂模式，隐藏了具体的实现类
 * 
 * @return std::unique_ptr<ProcessingPipelineInterface> 指向处理管道实例的唯一指针
 * @post 返回的处理管道实例已初始化并准备使用
 * @note 调用者负责管理返回的实例生命周期
 */
auto make_processing_pipeline() -> std::unique_ptr<ProcessingPipelineInterface>;

using IProcessingPipeline = ProcessingPipelineInterface;
auto create_processing_pipeline() -> std::unique_ptr<ProcessingPipelineInterface>;

} // namespace fq::processing


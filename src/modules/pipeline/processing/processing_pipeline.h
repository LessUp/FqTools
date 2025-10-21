#pragma once

/**
 * @file processing_pipeline.h
 * @brief FastQ 数据处理管道实现
 * @details 该模块实现了 FastQ 文件的读取、处理和写入功能，支持多种数据处理策略
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "fqtools/pipeline/processing_pipeline_interface.h"
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>

namespace fq::fastq {
    class FqInfoBatch;
    struct FqInfo;
}

namespace fq::processing {

class ReadMutatorInterface;
class ReadPredicateInterface;

/**
 * @brief 处理统计信息结构体
 * @details 记录 FastQ 数据处理过程中的各项统计指标，用于性能监控和结果分析
 * 
 * @invariant total_reads = passed_reads + filtered_reads + error_reads
 * @note 所有计数器均为原子操作，确保线程安全
 */
struct ProcessingStatistics {
    uint64_t total_reads = 0;          ///< 总读取数
    uint64_t passed_reads = 0;         ///< 通过筛选的读取数
    uint64_t filtered_reads = 0;       ///< 被过滤的读取数
    uint64_t modified_reads = 0;       ///< 被修改的读取数
    uint64_t error_reads = 0;          ///< 出错的读取数
    double processing_time_ms = 0.0;   ///< 处理时间（毫秒）
    double throughput_mbps = 0.0;      ///< 吞吐量（MB/s）
    
    /**
     * @brief 计算通过率
     * @return 通过率（0.0-1.0）
     */
    auto getPassRate() const -> double {
        return total_reads > 0 ? static_cast<double>(passed_reads) / total_reads : 0.0;
    }
    
    /**
     * @brief 计算过滤率
     * @return 过滤率（0.0-1.0）
     */
    auto getFilterRate() const -> double {
        return total_reads > 0 ? static_cast<double>(filtered_reads) / total_reads : 0.0;
    }
    
    /**
     * @brief 将统计信息转换为字符串
     * @return 格式化的统计信息字符串
     */
    auto toString() const -> std::string;
};


/**
 * @brief FastQ 数据处理管道实现类
 * @details 该类实现了 FastQ 文件的完整处理流程，包括：
 *          - 文件读取和解析
 *          - 数据过滤和转换
 *          - 并行处理支持
 *          - 结果写入和统计
 * 
 * 支持串行和并行（TBB）两种处理模式，可根据配置自动选择最优策略
 * 
 * @invariant 配置设置必须在运行前完成
 * @warning 处理大文件时建议使用并行模式以提高性能
 */
class SequentialProcessingPipeline : public ProcessingPipelineInterface {
public:
    /**
     * @brief 默认构造函数
     * @details 初始化处理管道，设置默认配置
     * @post 管道处于就绪状态，可以接受配置
     */
    SequentialProcessingPipeline();
    
    /**
     * @brief 析构函数
     * @details 清理所有资源，确保文件句柄正确关闭
     */
    ~SequentialProcessingPipeline();

    /**
     * @brief 设置输入文件路径
     * @details 指定要处理的 FastQ 文件路径
     * 
     * @param input_path 输入文件路径
     * @pre input_path 必须指向有效的 FastQ 文件
     * @post 输入路径被设置，但文件不会立即打开
     * @throw std::invalid_argument 如果路径为空
     */
    void setInput(const std::string& input_path) override;
    
    /**
     * @brief 设置输出文件路径
     * @details 指定处理结果的输出文件路径
     * 
     * @param output_path 输出文件路径
     * @pre output_path 必须是有效的文件路径
     * @post 输出路径被设置，但文件不会立即创建
     * @throw std::invalid_argument 如果路径为空
     */
    void setOutput(const std::string& output_path) override;
    
    /**
     * @brief 设置处理配置
     * @details 配置处理参数，包括线程数、批处理大小等
     * 
     * @param config 处理配置结构体
     * @pre config 必须包含有效的配置参数
     * @post 配置被应用到处理管道
     * @throw std::invalid_argument 如果配置参数无效
     */
    void setConfig(const ProcessingConfig& config) override;
    
    /**
     * @brief 添加数据修改器
     * @details 注册一个数据修改器，用于对读取数据进行转换
     * 
     * @param mutator 数据修改器的唯一指针
     * @pre mutator 必须是有效的修改器实例
     * @post 修改器被添加到处理链中
     * @note 修改器按添加顺序依次执行
     */
    void addMutator(std::unique_ptr<ReadMutatorInterface> mutator) override;
    
    /**
     * @brief 添加数据过滤器
     * @details 注册一个数据过滤器，用于筛选符合条件的读取
     * 
     * @param predicate 数据过滤器的唯一指针
     * @pre predicate 必须是有效的过滤器实例
     * @post 过滤器被添加到处理链中
     * @note 过滤器按添加顺序依次执行，任一过滤器失败则数据被过滤
     */
    void addPredicate(std::unique_ptr<ReadPredicateInterface> predicate) override;
    
    /**
     * @brief 执行数据处理
     * @details 启动完整的 FastQ 数据处理流程
     * 
     * @return ProcessingStatistics 处理统计信息
     * @pre 输入路径、输出路径和配置必须已设置
     * @post 处理结果写入输出文件，统计信息返回
     * @throw std::runtime_error 如果处理过程中发生错误
     * @note 处理时间取决于文件大小和配置参数
     */
    auto run() -> ProcessingStatistics override;

private:
    /**
     * @brief 串行处理模式
     * @details 在单线程中处理所有数据，适用于小文件或调试场景
     * 
     * @return ProcessingStatistics 处理统计信息
     * @pre 输入输出路径和配置必须已设置
     * @post 处理结果写入输出文件
     * @note 该方法较慢但便于调试
     */
    auto processSequential() -> ProcessingStatistics;
    
    /**
     * @brief 并行处理模式（使用 TBB）
     * @details 使用 Intel TBB 库进行并行处理，适用于大文件
     * 
     * @return ProcessingStatistics 处理统计信息
     * @pre 输入输出路径和配置必须已设置
     * @post 处理结果写入输出文件
     * @note 该方法性能最佳，适合生产环境
     */
    auto processWithTBB() -> ProcessingStatistics;
    
    /**
     * @brief 处理数据批次
     * @details 对一批 FastQ 数据进行处理，应用所有修改器和过滤器
     * 
     * @param batch 要处理的数据批次
     * @param stats 统计信息引用
     * @return bool 处理成功返回 true，失败返回 false
     * @pre batch 必须包含有效的数据
     * @post 统计信息被更新
     * @note 该方法是线程安全的，可在并行环境中调用
     */
    auto processBatch(fq::fastq::FqInfoBatch& batch, ProcessingStatistics& stats) -> bool;
    
    /**
     * @brief 处理单个读取
     * @details 对单个 FastQ 读取进行处理，应用所有修改器和过滤器
     * 
     * @param read 要处理的读取数据
     * @param stats 统计信息引用
     * @return bool 处理成功返回 true，失败返回 false
     * @pre read 必须包含有效的数据
     * @post 统计信息被更新
     * @note 该方法是线程安全的，可在并行环境中调用
     */
    auto processRead(fq::fastq::FqInfo& read, ProcessingStatistics& stats) -> bool;

    std::string m_input_path;                                      ///< 输入文件路径
    std::string m_output_path;                                     ///< 输出文件路径
    ProcessingConfig m_config;                                     ///< 处理配置
    std::vector<std::unique_ptr<ReadMutatorInterface>> m_mutators;         ///< 数据修改器列表
    std::vector<std::unique_ptr<ReadPredicateInterface>> m_predicates;     ///< 数据过滤器列表
};

}

#pragma once

#include "Processing/ProcessingPipeline.h"
#include "memory/BatchMemoryManager.h"
#include <memory>
#include <tbb/parallel_pipeline.h>

namespace fq::processing
{

/**
 * @brief 基于TBB的高性能并发处理流水线
 *
 * 这个类实现了设计文档中描述的三阶段解耦流水线：
 * 1. 输入阶段：读取和解析数据
 * 2. 处理阶段：并行应用所有处理器
 * 3. 输出阶段：格式化和写入数据
 *
 * 特点：
 * - 使用内存管理器重用FqInfoBatch对象
 * - 实现I/O与计算的重叠执行
 * - 自动背压控制
 * - 完善的错误处理和恢复机制
 */
class TbbProcessingPipeline : public IProcessingPipeline
{
public:
    /**
     * @brief 流水线配置
     */
    struct Config
    {
        size_t max_tokens;        ///< 流水线中最大令牌数
        size_t batch_size;        ///< 批处理大小
        size_t thread_count;      ///< 0表示使用硬件并发数
        bool enable_memory_pool;  ///< 启用内存池
        bool enable_backpressure; ///< 启用背压控制
        bool enable_statistics;   ///< 启用统计收集
        size_t memory_pool_size;  ///< 内存池大小

        Config()
            : max_tokens(16), batch_size(10000), thread_count(0), enable_memory_pool(true), enable_backpressure(true),
              enable_statistics(true), memory_pool_size(50)
        {
        }
    };

    /**
     * @brief 构造函数
     * @param config 流水线配置
     * @param memory_manager 内存管理器（可选，不提供则创建默认的）
     */
    explicit TbbProcessingPipeline(const Config &config = Config{},
                                   std::shared_ptr<fq::memory::BatchMemoryManager> memory_manager = nullptr);

    ~TbbProcessingPipeline() override;

    // IProcessingPipeline 接口实现
    void setInput(const std::string &input_path) override;
    void setOutput(const std::string &output_path) override;
    void setConfig(const ProcessingConfig &config) override;
    void addMutator(std::unique_ptr<IReadMutator> mutator) override;
    void addPredicate(std::unique_ptr<IReadPredicate> predicate) override;
    auto run() -> ProcessingStatistics override;

    /**
     * @brief 获取详细的性能统计
     */
    struct PerformanceStats
    {
        double total_time_ms = 0.0;
        double input_time_ms = 0.0;
        double processing_time_ms = 0.0;
        double output_time_ms = 0.0;
        uint64_t total_batches = 0;
        uint64_t total_reads = 0;
        double throughput_mbps = 0.0;
        double throughput_reads_per_sec = 0.0;
        double cpu_utilization = 0.0;
        size_t peak_memory_mb = 0;

        // 内存池统计
        struct MemoryPoolStats
        {
            size_t pool_size = 0;
            size_t active_count = 0;
            size_t hit_count = 0;
            size_t miss_count = 0;
            double hit_rate = 0.0;
        } memory_pool_stats;
    };

    /**
     * @brief 获取性能统计
     */
    [[nodiscard]] auto get_performance_stats() const -> PerformanceStats;

    /**
     * @brief 重置统计信息
     */
    void reset_stats();

private:
    // 流水线配置
    Config m_pipeline_config;
    ProcessingConfig m_processing_config;

    // 内存管理
    std::shared_ptr<fq::memory::BatchMemoryManager> m_memory_manager;
    bool m_owns_memory_manager = false;

    // 处理器
    std::vector<std::unique_ptr<IReadMutator>> m_mutators;
    std::vector<std::unique_ptr<IReadPredicate>> m_predicates;

    // 文件路径
    std::string m_input_path;
    std::string m_output_path;

    // 性能统计
    mutable std::mutex m_stats_mutex;
    PerformanceStats m_stats;

    // 私有方法
    void initialize_memory_manager();
    void validate_config() const;

    // 流水线阶段实现
    class InputFilter;
    class ProcessingFilter;
    class OutputFilter;

    friend class InputFilter;
    friend class ProcessingFilter;
    friend class OutputFilter;

    // 统计更新方法
    void update_input_stats(double duration_ms, size_t reads_count);
    void update_processing_stats(double duration_ms, size_t reads_count);
    void update_output_stats(double duration_ms);
    void finalize_stats();
};

/**
 * @brief 创建TBB处理流水线的工厂函数
 */
auto create_tbb_pipeline(const TbbProcessingPipeline::Config &config = TbbProcessingPipeline::Config{},
                         std::shared_ptr<fq::memory::BatchMemoryManager> memory_manager = nullptr)
    -> std::unique_ptr<IProcessingPipeline>;

} // namespace fq::processing

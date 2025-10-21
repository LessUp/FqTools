/**
 * @file tbb_processing_pipeline.cpp
 * @brief TBB 并行处理流水线实现文件
 * @details 实现了 TbbProcessingPipeline 类的构造、析构及主要接口，支持高效并行处理。
 * @author FastQTools Team
 * @date 2025-08-01
 * @version 1.0
 * @copyright Copyright (c) 2025 FastQTools
 */

#include "processing/tbb_processing_pipeline.h"
#include "processing/i_read_processor.h"
#include "core_legacy/core.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <limits>

namespace fq::processing {

//==============================================================================
// TbbProcessingPipeline Implementation
//==============================================================================

/**
 * @brief TbbProcessingPipeline 构造函数
 * @details 初始化流水线配置和内存管理器
 * @param config 流水线配置
 * @param memory_manager 内存管理器指针
 */
TbbProcessingPipeline::TbbProcessingPipeline(
    const Config& config,
    std::shared_ptr<fq::memory::BatchMemoryManager> memory_manager
) : m_pipeline_config(config), m_memory_manager(std::move(memory_manager)) {
    
    initialize_memory_manager();
    validate_config();
}

/**
 * @brief TbbProcessingPipeline 析构函数
 * @details 负责清理全局内存管理器（如需要）
 */
TbbProcessingPipeline::~TbbProcessingPipeline() {
    if (m_owns_memory_manager && m_memory_manager) {
        fq::memory::cleanup_global_memory_manager();
    }
}

/**
 * @brief 设置输入文件路径
 * @param input_path 输入文件路径
 */
void TbbProcessingPipeline::setInput(const std::string& input_path) {
    m_input_path = input_path;
}

/**
 * @brief 设置输出文件路径
 * @param output_path 输出文件路径
 */
void TbbProcessingPipeline::setOutput(const std::string& output_path) {
    m_output_path = output_path;
}

/**
 * @brief 设置流水线处理配置
 * @param config 处理配置参数
 */
void TbbProcessingPipeline::setConfig(const ProcessingConfig& config) {
    m_processing_config = config;
    
    // 更新流水线配置
    if (config.thread_count > 0) {
        m_pipeline_config.thread_count = config.thread_count;
    }
    if (config.batch_size > 0) {
        m_pipeline_config.batch_size = config.batch_size;
    }
}

/**
 * @brief 添加数据修改器
 * @param mutator 读数据修改器
 */
void TbbProcessingPipeline::addMutator(std::unique_ptr<IReadMutator> mutator) {
    m_mutators.push_back(std::move(mutator));
}

/**
 * @brief 添加数据过滤器
 * @param predicate 读数据过滤器
 */
void TbbProcessingPipeline::addPredicate(std::unique_ptr<IReadPredicate> predicate) {
    m_predicates.push_back(std::move(predicate));
}

/**
 * @brief 启动流水线并返回处理统计信息
 * @return 处理统计结果
 */
auto TbbProcessingPipeline::run() -> ProcessingStatistics {
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        // 验证配置
        if (m_input_path.empty()) {
            throw fq::exception("Input path not set");
        }
        if (m_output_path.empty()) {
            throw fq::exception("Output path not set");
        }
        
        spdlog::info("Starting TBB processing pipeline");
        spdlog::info("Input: {}", m_input_path);
        spdlog::info("Output: {}", m_output_path);
        spdlog::info("Batch size: {}", m_pipeline_config.batch_size);
        spdlog::info("Max tokens: {}", m_pipeline_config.max_tokens);
        spdlog::info("Thread count: {}", m_pipeline_config.thread_count > 0 ? 
                     m_pipeline_config.thread_count : std::thread::hardware_concurrency());
        
        // 重置统计
        reset_stats();
        
        // 最终统计结果
        ProcessingStatistics final_stats;
        std::atomic<uint64_t> total_reads_processed{0};
        std::atomic<uint64_t> total_reads_passed{0};
        
        // 计算实际的线程数
        size_t actual_thread_count = m_pipeline_config.thread_count > 0 ? 
                                   m_pipeline_config.thread_count : 
                                   std::thread::hardware_concurrency();
        
        spdlog::info("Using {} threads for processing", actual_thread_count);
        
        // TBB并行流水线实现
        tbb::parallel_pipeline(
            m_pipeline_config.max_tokens,
            
            // 阶段1：输入过滤器 (串行)
            tbb::make_filter<void, std::unique_ptr<fq::fastq::FqInfoBatch>>(
                tbb::filter_mode::serial_in_order,
                [this, &total_reads_processed](tbb::flow_control& fc) -> std::unique_ptr<fq::fastq::FqInfoBatch> {
                    auto stage_start = std::chrono::steady_clock::now();
                    
                    try {
                        // 从内存池获取批处理对象
                        auto batch = m_memory_manager->acquire_batch();
                        batch->reads.reserve(m_pipeline_config.batch_size);
                        
                        // 使用线程局部的读取器
                        thread_local static std::unique_ptr<fq::fastq::FastQReader> reader;
                        if (!reader) {
                            reader = std::make_unique<fq::fastq::FastQReader>(m_input_path);
                            if (!reader->isOpened()) {
                                throw fq::exception("Failed to open input file: " + m_input_path);
                            }
                        }
                        
                        if (m_pipeline_config.batch_size > std::numeric_limits<int>::max()) {
                            throw fq::exception("Batch size exceeds the maximum value for an integer.");
                        }
                        // 读取数据
                        if (reader->read(*batch, static_cast<int>(m_pipeline_config.batch_size))) {
                            auto stage_end = std::chrono::steady_clock::now();
                            auto duration = std::chrono::duration<double, std::milli>(stage_end - stage_start).count();
                            
                            total_reads_processed += batch->size();
                            update_input_stats(duration, batch->size());
                            
                            return batch;
                        } else {
                            // 文件读取完成
                            fc.stop();
                            m_memory_manager->release_batch(std::move(batch));
                            return nullptr;
                        }
                    } catch (const std::exception& e) {
                        spdlog::error("Error in input filter: {}", e.what());
                        fc.stop();
                        throw;
                    }
                }
            ) &
            
            // 阶段2：处理过滤器 (并行)
            tbb::make_filter<std::unique_ptr<fq::fastq::FqInfoBatch>, std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics>>(
                tbb::filter_mode::parallel,
                [this, &total_reads_passed](std::unique_ptr<fq::fastq::FqInfoBatch> batch) -> std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics> {
                    auto stage_start = std::chrono::steady_clock::now();
                    ProcessingStatistics batch_stats;
                    
                    try {
                        // 预分配通过筛选的reads
                        std::vector<fq::fastq::FqInfo> passed_reads;
                        passed_reads.reserve(batch->reads.size());
                        
                        // 处理每个read
                        for (auto& read : batch->reads) {
                            batch_stats.total_reads++;
                            
                            // 应用所有谓词
                            bool passed = true;
                            for (const auto& predicate : m_predicates) {
                                if (!predicate->evaluate(read)) {
                                    passed = false;
                                    batch_stats.filtered_reads++;
                                    break;
                                }
                            }
                            
                            // 如果通过谓词，应用所有处理器
                            if (passed) {
                                for (const auto& mutator : m_mutators) {
                                    mutator->process(read);
                                    batch_stats.modified_reads++;
                                }
                                passed_reads.push_back(std::move(read));
                                batch_stats.passed_reads++;
                            }
                        }
                        
                        // 更新批次数据
                        batch->reads = std::move(passed_reads);
                        
                        auto stage_end = std::chrono::steady_clock::now();
                        auto duration = std::chrono::duration<double, std::milli>(stage_end - stage_start).count();
                        
                        total_reads_passed += batch_stats.passed_reads;
                        update_processing_stats(duration, batch_stats.total_reads);
                        
                        return std::make_pair(std::move(batch), batch_stats);
                        
                    } catch (const std::exception& e) {
                        spdlog::error("Error in processing filter: {}", e.what());
                        throw;
                    }
                }
            ) &
            
            // 阶段3：输出过滤器 (串行)
            tbb::make_filter<std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics>, void>(
                tbb::filter_mode::serial_in_order,
                [this, &final_stats](std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics>&& result) {
                    auto stage_start = std::chrono::steady_clock::now();
                    
                    try {
                        const auto& batch = result.first;
                        const auto& batch_stats = result.second;
                        
                        // 使用线程局部的写入器
                        thread_local static std::unique_ptr<fq::fastq::FastQWriter> writer;
                        if (!writer) {
                            writer = std::make_unique<fq::fastq::FastQWriter>(m_output_path);
                            if (!writer->isOpened()) {
                                throw fq::exception("Failed to open output file: " + m_output_path);
                            }
                        }
                        
                        // 写入数据
                        writer->write(*batch);
                        
                        // 释放批处理对象回内存池
                        m_memory_manager->release_batch(std::move(result.first));
                        
                        // 累加统计信息
                        final_stats.total_reads += batch_stats.total_reads;
                        final_stats.passed_reads += batch_stats.passed_reads;
                        final_stats.filtered_reads += batch_stats.filtered_reads;
                        final_stats.modified_reads += batch_stats.modified_reads;
                        
                        auto stage_end = std::chrono::steady_clock::now();
                        auto duration = std::chrono::duration<double, std::milli>(stage_end - stage_start).count();
                        
                        update_output_stats(duration);
                        
                    } catch (const std::exception& e) {
                        spdlog::error("Error in output filter: {}", e.what());
                        throw;
                    }
                }
            )
        );
        
        // 完成统计
        auto end_time = std::chrono::steady_clock::now();
        auto total_duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        final_stats.processing_time_ms = total_duration;
        final_stats.total_reads = total_reads_processed.load();
        final_stats.passed_reads = total_reads_passed.load();
        
                constexpr double ESTIMATED_AVG_READ_LENGTH = 150.0;
        constexpr double MS_PER_SECOND = 1000.0;
        constexpr double BYTES_PER_KB = 1024.0;
        constexpr double BYTES_PER_MB = BYTES_PER_KB * BYTES_PER_KB;
        // 计算吞吐量
        if (total_duration > 0) {
                            final_stats.throughput_mbps = (static_cast<double>(total_reads_processed.load()) * ESTIMATED_AVG_READ_LENGTH) / (total_duration / MS_PER_SECOND) / (BYTES_PER_MB);
        }
        
        finalize_stats();
        
        spdlog::info("TBB pipeline completed successfully");
        spdlog::info("Total time: {:.2f} ms", total_duration);
        spdlog::info("Total reads: {}", final_stats.total_reads);
        spdlog::info("Passed reads: {}", final_stats.passed_reads);
        spdlog::info("Filtered reads: {}", final_stats.filtered_reads);
        spdlog::info("Throughput: {:.2f} MB/s", final_stats.throughput_mbps);
        
        return final_stats;
        
    } catch (const std::exception& e) {
        spdlog::error("TBB pipeline failed: {}", e.what());
        throw;
    }
}

auto TbbProcessingPipeline::get_performance_stats() const -> TbbProcessingPipeline::PerformanceStats {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    // 创建统计副本
    PerformanceStats stats = m_stats;
    
    // 获取内存池统计
    if (m_memory_manager && m_pipeline_config.enable_memory_pool) {
        auto pool_stats = m_memory_manager->get_batch_pool_stats();
        stats.memory_pool_stats.pool_size = pool_stats.pool_size;
        stats.memory_pool_stats.active_count = pool_stats.active_count;
        stats.memory_pool_stats.hit_count = pool_stats.hit_count;
        stats.memory_pool_stats.miss_count = pool_stats.miss_count;
        stats.memory_pool_stats.hit_rate = pool_stats.hit_count + pool_stats.miss_count > 0 ? 
            static_cast<double>(pool_stats.hit_count) / (pool_stats.hit_count + pool_stats.miss_count) : 0.0;
    }
    
    return stats;
}

void TbbProcessingPipeline::reset_stats() {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_stats = PerformanceStats{};
}

void TbbProcessingPipeline::initialize_memory_manager() {
    if (!m_memory_manager && m_pipeline_config.enable_memory_pool) {
        fq::memory::BatchMemoryManager::Config config;
        config.initial_batch_pool_size = m_pipeline_config.memory_pool_size;
        config.max_batch_pool_size = m_pipeline_config.memory_pool_size * 2;
        config.enable_auto_shrink = true;
        config.enable_stats = true;
        
        fq::memory::init_global_memory_manager(config);
        m_memory_manager = fq::memory::global_memory_manager();
        m_owns_memory_manager = true;
    }
}

void TbbProcessingPipeline::validate_config() const {
    if (m_pipeline_config.max_tokens < 1) {
        throw fq::exception("Max tokens must be at least 1");
    }
    if (m_pipeline_config.batch_size < 1) {
        throw fq::exception("Batch size must be at least 1");
    }
}

void TbbProcessingPipeline::update_input_stats(double duration_ms, size_t reads_count) {
    if (!m_pipeline_config.enable_statistics) return;
    
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_stats.input_time_ms += duration_ms;
    m_stats.total_batches++;
    m_stats.total_reads += reads_count;
}

void TbbProcessingPipeline::update_processing_stats(double duration_ms, size_t reads_count) {
    if (!m_pipeline_config.enable_statistics) return;
    
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_stats.processing_time_ms += duration_ms;
    // reads_count 参数可用于未来的统计扩展
    (void)reads_count; // 避免未使用参数的警告
}

void TbbProcessingPipeline::update_output_stats(double duration_ms) {
    if (!m_pipeline_config.enable_statistics) return;
    
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_stats.output_time_ms += duration_ms;
}

void TbbProcessingPipeline::finalize_stats() {
    if (!m_pipeline_config.enable_statistics) return;
    
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    m_stats.total_time_ms = m_stats.input_time_ms + m_stats.processing_time_ms + m_stats.output_time_ms;
    
    // 计算吞吐量
    if (m_stats.total_time_ms > 0) {
                        constexpr double ESTIMATED_AVG_READ_LENGTH = 150.0;
        constexpr double MS_PER_SECOND = 1000.0;
        constexpr double BYTES_PER_KB = 1024.0;
        constexpr double BYTES_PER_MB = BYTES_PER_KB * BYTES_PER_KB;
        m_stats.throughput_reads_per_sec = (static_cast<double>(m_stats.total_reads) * MS_PER_SECOND) / m_stats.total_time_ms;
        
        // 估算吞吐量 (假设平均read长度为150bp)
        m_stats.throughput_mbps = (static_cast<double>(m_stats.total_reads) * ESTIMATED_AVG_READ_LENGTH) / (m_stats.total_time_ms / MS_PER_SECOND) / (BYTES_PER_MB);
    }
    
    // 估算CPU利用率
    double total_cpu_time = m_stats.input_time_ms + m_stats.processing_time_ms + m_stats.output_time_ms;
    if (m_stats.total_time_ms > 0) {
        m_stats.cpu_utilization = std::min(100.0, (total_cpu_time / m_stats.total_time_ms) * 100.0);
    }
    
    // 获取峰值内存使用
    if (m_memory_manager) {
        // 将字节转换为 MB（向下取整）
        constexpr size_t BYTES_PER_MB_SZ = 1024 * 1024;
        m_stats.peak_memory_mb = m_memory_manager->get_memory_usage() / BYTES_PER_MB_SZ;
    }
}

//==============================================================================
// factory Function Implementation
//==============================================================================

auto create_tbb_pipeline(
    const TbbProcessingPipeline::Config& config,
    std::shared_ptr<fq::memory::BatchMemoryManager> memory_manager
) -> std::unique_ptr<IProcessingPipeline> {
    return std::make_unique<TbbProcessingPipeline>(config, memory_manager);
}

} // namespace fq::processing

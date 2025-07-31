#include "Processing/ProcessingPipeline.h"
#include "Processing/IReadProcessor.h"
#include "Core/Core.h"
#include <iostream>
#include <tbb/parallel_pipeline.h>
#include <tbb/concurrent_vector.h>
#include <memory>

namespace fq::processing {

ProcessingPipeline::ProcessingPipeline() = default;
ProcessingPipeline::~ProcessingPipeline() = default;

void ProcessingPipeline::setInput(const std::string& input_path) { m_input_path = input_path; }
void ProcessingPipeline::setOutput(const std::string& output_path) { m_output_path = output_path; }
void ProcessingPipeline::setConfig(const ProcessingConfig& config) { m_config = config; }
void ProcessingPipeline::addMutator(std::unique_ptr<IReadMutator> mutator) { m_mutators.push_back(std::move(mutator)); }
void ProcessingPipeline::addPredicate(std::unique_ptr<IReadPredicate> predicate) { m_predicates.push_back(std::move(predicate)); }

auto ProcessingPipeline::run() -> ProcessingStatistics {
    // 如果配置的线程数大于1，使用TBB并行流水线
    if (m_config.thread_count > 1) {
        return processWithTBB();
    } else {
        return processSequential();
    }
}

auto ProcessingPipeline::processSequential() -> ProcessingStatistics {
    ProcessingStatistics stats;
    fq::fastq::FastQReader reader(m_input_path);
    fq::fastq::FastQWriter writer(m_output_path);

    if (!reader.isOpened()) throw fq::exception("Failed to open input file: " + m_input_path);
    if (!writer.isOpened()) throw fq::exception("Failed to open output file: " + m_output_path);

    fq::fastq::FqInfoBatch batch;
    while (reader.read(batch, m_config.batch_size)) {
        processBatch(batch, stats);
        writer.write(batch);
    }
    return stats;
}

auto ProcessingPipeline::processBatch(fq::fastq::FqInfoBatch& batch, ProcessingStatistics& stats) -> bool {
    std::vector<fq::fastq::FqInfo> passed_reads;
    passed_reads.reserve(batch.reads.size());

    for (auto& read : batch.reads) {
        stats.total_reads++;
        bool passed = true;
        for (const auto& predicate : m_predicates) {
            if (!predicate->evaluate(read)) {
                passed = false;
                break;
            }
        }

        if (passed) {
            for (const auto& mutator : m_mutators) {
                mutator->process(read);
            }
            passed_reads.push_back(std::move(read));
        }
    }
    stats.passed_reads += passed_reads.size();
    batch.reads = std::move(passed_reads);
    return true;
}

auto ProcessingPipeline::processWithTBB() -> ProcessingStatistics {
    ProcessingStatistics final_stats;
    
    // 性能监控
    auto start_time = std::chrono::steady_clock::now();
    std::atomic<uint64_t> batches_processed{0};
    std::atomic<uint64_t> reads_processed{0};
    
    try {
        // 计算最优的max_tokens值：基于线程数和系统资源
        size_t max_tokens = std::max(static_cast<size_t>(4), m_config.thread_count * 2);
        
        spdlog::info("Starting TBB parallel pipeline with {} threads, max_tokens: {}", 
                    m_config.thread_count, max_tokens);
        
        // TBB并行流水线实现
        tbb::parallel_pipeline(
            max_tokens,
            
            // 阶段1：输入过滤器 (串行)
            tbb::make_filter<void, std::unique_ptr<fq::fastq::FqInfoBatch>>(
                tbb::filter_mode::serial_in_order,
                [&](tbb::flow_control& flow_control) -> std::unique_ptr<fq::fastq::FqInfoBatch> {
                    try {
                        auto batch = std::make_unique<fq::fastq::FqInfoBatch>();
                        
                        // 使用静态reader，避免重复打开文件
                        thread_local static std::unique_ptr<fq::fastq::FastQReader> reader;
                        if (!reader) {
                            reader = std::make_unique<fq::fastq::FastQReader>(m_input_path);
                            if (!reader->isOpened()) {
                                throw fq::exception("Failed to open input file: " + m_input_path);
                            }
                        }
                        
                        if (reader->read(*batch, m_config.batch_size)) {
                            batches_processed++;
                            reads_processed += batch->size();
                            return batch;
                        } else {
                            // 文件读取完成，停止流水线
                            flow_control.stop();
                            return nullptr;
                        }
                    } catch (const std::exception& e) {
                        spdlog::error("Error in input filter: {}", e.what());
                        flow_control.stop();
                        throw;
                    }
                }
            ) &
            
            // 阶段2：处理过滤器 (并行)
            tbb::make_filter<std::unique_ptr<fq::fastq::FqInfoBatch>, std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics>>(
                tbb::filter_mode::parallel,
                [&](std::unique_ptr<fq::fastq::FqInfoBatch> batch) -> std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics> {
                    ProcessingStatistics batch_stats;
                    
                    try {
                        // 处理批次中的每个read
                        std::vector<fq::fastq::FqInfo> passed_reads;
                        passed_reads.reserve(batch->reads.size());
                        
                        for (auto& read : batch->reads) {
                            batch_stats.total_reads++;
                            
                            // 应用所有谓词
                            bool passed = true;
                            for (const auto& predicate : m_predicates) {
                                if (!predicate->evaluate(read)) {
                                    passed = false;
                                    break;
                                }
                            }
                            
                            // 如果通过谓词，应用所有处理器
                            if (passed) {
                                for (const auto& mutator : m_mutators) {
                                    mutator->process(read);
                                }
                                passed_reads.push_back(std::move(read));
                            }
                        }
                        
                        batch_stats.passed_reads += passed_reads.size();
                        batch->reads = std::move(passed_reads);
                        
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
                [&](const std::pair<std::unique_ptr<fq::fastq::FqInfoBatch>, ProcessingStatistics>& result) {
                    try {
                        const auto& batch = result.first;
                        const auto& batch_stats = result.second;
                        
                        // 使用静态writer，避免重复打开文件
                        thread_local static std::unique_ptr<fq::fastq::FastQWriter> writer;
                        if (!writer) {
                            writer = std::make_unique<fq::fastq::FastQWriter>(m_output_path);
                            if (!writer->isOpened()) {
                                throw fq::exception("Failed to open output file: " + m_output_path);
                            }
                        }
                        
                        // 写入处理后的批次
                        writer->write(*batch);
                        
                        // 累加统计信息
                        final_stats.total_reads += batch_stats.total_reads;
                        final_stats.passed_reads += batch_stats.passed_reads;
                        
                    } catch (const std::exception& e) {
                        spdlog::error("Error in output filter: {}", e.what());
                        throw;
                    }
                }
            )
        );
        
        // 性能统计
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
        
        spdlog::info("TBB pipeline completed:");
        spdlog::info("  Duration: {} seconds", duration);
        spdlog::info("  Batches processed: {}", batches_processed.load());
        spdlog::info("  Reads processed: {}", reads_processed.load());
        spdlog::info("  Total reads: {}", final_stats.total_reads);
        spdlog::info("  Passed reads: {}", final_stats.passed_reads);
        spdlog::info("  Throughput: {} reads/sec", duration > 0 ? reads_processed.load() / duration : 0);
        
    } catch (const std::exception& e) {
        spdlog::error("TBB pipeline failed: {}", e.what());
        // 发生错误时，回退到顺序处理
        spdlog::info("Falling back to sequential processing");
        return processSequential();
    }
    
    return final_stats;
}

} // namespace fq::processing

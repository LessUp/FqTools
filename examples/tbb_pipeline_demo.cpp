#include "interfaces/i_processing_pipeline.h"
#include "processing/tbb_processing_pipeline.h"
#include "core_legacy/core.h"
#include <iostream>
#include <chrono>

using namespace fq::processing;

int main() {
    try {
        std::cout << "FastQTools 高性能流水线演示" << std::endl;
        
        // 创建TBB高性能流水线
        TbbProcessingPipeline::Config config;
        config.max_tokens = 16;
        config.batch_size = 10000;
        config.thread_count = 0; // 使用硬件并发数
        config.enable_memory_pool = true;
        config.enable_statistics = true;
        config.memory_pool_size = 50;
        
        auto pipeline = create_tbb_pipeline(config);
        
        // 设置输入输出文件
        pipeline->setInput("input.fastq.gz");
        pipeline->setOutput("output.fastq.gz");
        
        // 设置处理配置
        ProcessingConfig proc_config;
        proc_config.batch_size = 10000;
        proc_config.thread_count = 8;
        pipeline->setConfig(proc_config);
        
        // 添加处理器（示例）
        // pipeline->addPredicate(std::make_unique<MinQualityPredicate>(20));
        // pipeline->addMutator(std::make_unique<QualityTrimmer>(20));
        
        std::cout << "开始处理..." << std::endl;
        auto start_time = std::chrono::steady_clock::now();
        
        // 运行流水线
        auto stats = pipeline->run();
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double>(end_time - start_time).count();
        
        std::cout << "处理完成！" << std::endl;
        std::cout << "处理时间: " << duration << " 秒" << std::endl;
        std::cout << "总读取数: " << stats.total_reads << std::endl;
        std::cout << "通过读取数: " << stats.passed_reads << std::endl;
        std::cout << "过滤读取数: " << stats.filtered_reads << std::endl;
        std::cout << "修改读取数: " << stats.modified_reads << std::endl;
        std::cout << "通过率: " << (stats.total_reads > 0 ? stats.getPassRate() * 100 : 0) << "%" << std::endl;
        std::cout << "吞吐量: " << stats.throughput_mbps << " MB/s" << std::endl;
        
        // 如果是TbbProcessingPipeline，获取详细性能统计
        auto* tbb_pipeline = dynamic_cast<TbbProcessingPipeline*>(pipeline.get());
        if (tbb_pipeline) {
            auto perf_stats = tbb_pipeline->get_performance_stats();
            std::cout << "\n详细性能统计:" << std::endl;
            std::cout << "输入阶段时间: " << perf_stats.input_time_ms << " ms" << std::endl;
            std::cout << "处理阶段时间: " << perf_stats.processing_time_ms << " ms" << std::endl;
            std::cout << "输出阶段时间: " << perf_stats.output_time_ms << " ms" << std::endl;
            std::cout << "CPU利用率: " << perf_stats.cpu_utilization << "%" << std::endl;
            std::cout << "峰值内存: " << perf_stats.peak_memory_mb << " MB" << std::endl;
            
            if (config.enable_memory_pool) {
                std::cout << "内存池命中率: " << (perf_stats.memory_pool_stats.hit_rate * 100) << "%" << std::endl;
                std::cout << "内存池大小: " << perf_stats.memory_pool_stats.pool_size << std::endl;
                std::cout << "活跃对象: " << perf_stats.memory_pool_stats.active_count << std::endl;
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}
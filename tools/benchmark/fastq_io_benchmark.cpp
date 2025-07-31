#include <benchmark/benchmark.h>
#include "fastq/FastQReader.h"
#include "fastq/FastQWriter.h"
#include "fastq/FastQ.h"
#include <vector>
#include <string>
#include <sstream>
#include <random>

namespace fq::benchmark {

// 生成测试数据的辅助函数
class TestDataGenerator {
public:
    static std::string generateFastQRecord(size_t read_length = 150) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> base_dis(0, 3);
        static std::uniform_int_distribution<> qual_dis(33, 73); // Phred+33
        
        static const char bases[] = "ATGC";
        
        std::ostringstream oss;
        oss << "@read_" << gen() << "\n";
        
        // 生成序列
        for (size_t i = 0; i < read_length; ++i) {
            oss << bases[base_dis(gen)];
        }
        oss << "\n+\n";
        
        // 生成质量分数
        for (size_t i = 0; i < read_length; ++i) {
            oss << static_cast<char>(qual_dis(gen));
        }
        oss << "\n";
        
        return oss.str();
    }
    
    static std::string generateFastQFile(size_t num_reads, size_t read_length = 150) {
        std::ostringstream oss;
        for (size_t i = 0; i < num_reads; ++i) {
            oss << generateFastQRecord(read_length);
        }
        return oss.str();
    }
};

// FastQ 读取基准测试
static void BM_FastQReader_SmallFile(::benchmark::State& state) {
    const size_t num_reads = state.range(0);
    std::string test_data = TestDataGenerator::generateFastQFile(num_reads);
    
    for (auto _ : state) {
        std::istringstream iss(test_data);
        fq::fastq::FastQReader reader;
        
        // 模拟从流中读取
        std::vector<fq::fastq::FqInfo> reads;
        fq::fastq::FqInfo read;
        
        size_t count = 0;
        while (count < num_reads) {
            // 这里需要实际的读取逻辑
            // reader.readNext(read);
            reads.push_back(read);
            count++;
        }
        
        ::benchmark::DoNotOptimize(reads);
    }
    
    state.SetItemsProcessed(state.iterations() * num_reads);
    state.SetBytesProcessed(state.iterations() * test_data.size());
}

// FastQ 写入基准测试
static void BM_FastQWriter_SmallFile(::benchmark::State& state) {
    const size_t num_reads = state.range(0);
    
    // 准备测试数据
    std::vector<fq::fastq::FqInfo> reads;
    for (size_t i = 0; i < num_reads; ++i) {
        fq::fastq::FqInfo read;
        read.name = "@read_" + std::to_string(i);
        read.seq = TestDataGenerator::generateFastQRecord(150).substr(10, 150); // 提取序列部分
        read.strand = "+";
        read.qual = std::string(150, 'I'); // 高质量分数
        reads.push_back(read);
    }
    
    for (auto _ : state) {
        std::ostringstream oss;
        fq::fastq::FastQWriter writer;
        
        for (const auto& read : reads) {
            // writer.write(read, oss);
            oss << read.name << "\n" << read.seq << "\n" << read.strand << "\n" << read.qual << "\n";
        }
        
        ::benchmark::DoNotOptimize(oss.str());
    }
    
    state.SetItemsProcessed(state.iterations() * num_reads);
}

// 批量读取基准测试
static void BM_FastQReader_BatchRead(::benchmark::State& state) {
    const size_t num_reads = state.range(0);
    const size_t batch_size = 1000;
    std::string test_data = TestDataGenerator::generateFastQFile(num_reads);
    
    for (auto _ : state) {
        std::istringstream iss(test_data);
        fq::fastq::FastQReader reader;
        
        std::vector<fq::fastq::FqInfo> batch;
        batch.reserve(batch_size);
        
        size_t total_read = 0;
        while (total_read < num_reads) {
            batch.clear();
            
            // 读取一批数据
            for (size_t i = 0; i < batch_size && total_read < num_reads; ++i) {
                fq::fastq::FqInfo read;
                // reader.readNext(read);
                batch.push_back(read);
                total_read++;
            }
            
            ::benchmark::DoNotOptimize(batch);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * num_reads);
    state.SetBytesProcessed(state.iterations() * test_data.size());
}

// 内存使用基准测试
static void BM_FastQReader_MemoryUsage(::benchmark::State& state) {
    const size_t num_reads = state.range(0);
    std::string test_data = TestDataGenerator::generateFastQFile(num_reads);
    
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<fq::fastq::FqInfo> reads;
        reads.reserve(num_reads);
        state.ResumeTiming();
        
        std::istringstream iss(test_data);
        fq::fastq::FastQReader reader;
        
        for (size_t i = 0; i < num_reads; ++i) {
            fq::fastq::FqInfo read;
            // reader.readNext(read);
            reads.push_back(std::move(read));
        }
        
        ::benchmark::DoNotOptimize(reads);
    }
    
    state.SetItemsProcessed(state.iterations() * num_reads);
}

// 注册基准测试
BENCHMARK(BM_FastQReader_SmallFile)
    ->Range(1000, 100000)
    ->Unit(::benchmark::kMicrosecond);

BENCHMARK(BM_FastQWriter_SmallFile)
    ->Range(1000, 100000)
    ->Unit(::benchmark::kMicrosecond);

BENCHMARK(BM_FastQReader_BatchRead)
    ->Range(10000, 1000000)
    ->Unit(::benchmark::kMillisecond);

BENCHMARK(BM_FastQReader_MemoryUsage)
    ->Range(1000, 50000)
    ->Unit(::benchmark::kMicrosecond);

} // namespace fq::benchmark

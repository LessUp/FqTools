# FastQTools 基准测试

cmake_minimum_required(VERSION 3.20)

# 查找 Google Benchmark
find_package(benchmark CONFIG REQUIRED)

# 设置基准测试通用配置
set(BENCHMARK_COMMON_LIBS
    benchmark::benchmark
    benchmark::benchmark_main
    fastq_common
    fastq_fastq
    fastq_statistics
    fastq_encoder
)

# 基准测试工具函数
function(add_benchmark benchmark_name source_files)
    add_executable(${benchmark_name} ${source_files})
    
    target_link_libraries(${benchmark_name} PRIVATE
        ${BENCHMARK_COMMON_LIBS}
    )
    
    target_include_directories(${benchmark_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # 设置基准测试属性
    set_target_properties(${benchmark_name} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

# FastQ 读写基准测试
add_benchmark(benchmark_fastq_io
    fastq_io_benchmark.cpp
)

# 统计分析基准测试
add_benchmark(benchmark_statistics
    statistics_benchmark.cpp
)

# 编码压缩基准测试
add_benchmark(benchmark_encoder
    encoder_benchmark.cpp
)

# 内存使用基准测试
add_benchmark(benchmark_memory
    memory_benchmark.cpp
)

# 端到端性能基准测试
add_executable(performance_benchmark
    performance_benchmark.cpp
)
target_link_libraries(performance_benchmark PRIVATE
    fq_statistics
    fq_encoder
    fq_fastq
    fq_common
    fq_processing
    cxxopts::cxxopts
    spdlog::spdlog
)

# 创建基准测试组
add_custom_target(benchmarks
    DEPENDS 
        benchmark_fastq_io
        benchmark_statistics
        benchmark_encoder
        benchmark_memory
    COMMENT "Building all benchmarks"
)

# 运行所有基准测试的目标
add_custom_target(run_benchmarks
    COMMAND benchmark_fastq_io --benchmark_format=json --benchmark_out=fastq_io_results.json
    COMMAND benchmark_statistics --benchmark_format=json --benchmark_out=statistics_results.json
    COMMAND benchmark_encoder --benchmark_format=json --benchmark_out=encoder_results.json
    COMMAND benchmark_memory --benchmark_format=json --benchmark_out=memory_results.json
    DEPENDS benchmarks
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all benchmarks"
)

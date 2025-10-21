add_library(fq_pipeline STATIC
    factory.cpp
    processing/processing_pipeline.cpp
    processing/tbb_processing_pipeline.cpp
    processing/i_read_processor.cpp
    processing/mutators/quality_trimmer.cpp
    processing/predicates/min_quality_predicate.cpp
    memory/batch_memory_manager.cpp
    statistics/fq_statistic.cpp
    statistics/fq_statistic_worker.cpp
)

target_include_directories(fq_pipeline
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/gzstream/include
)

target_link_libraries(fq_pipeline
    PUBLIC
        fq_legacy_lib
        fq_core
        fq_io
        fq_fastq
        spdlog::spdlog
        TBB::tbb
        fmt::fmt
)

target_compile_features(fq_pipeline PUBLIC cxx_std_20)

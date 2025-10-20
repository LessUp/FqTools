# Add the new C++20 modules
add_subdirectory(modules)

# Add the command line interface library
add_subdirectory(cli)

# For now, group all non-module code into a temporary "legacy" library.
# We will break this down into smaller modules later.
add_library(fq_legacy_lib STATIC
    core_legacy/core.h
    core_legacy/std.h
    core_legacy/core.cpp
    interfaces/i_statistic_calculator.h
    interfaces/i_processing_pipeline.h
    interfaces/factory.cpp
    statistics/fq_statistic_worker.cpp
    statistics/fq_statistic.cpp
    processing/processing_pipeline.cpp
    processing/tbb_processing_pipeline.cpp
    processing/mutators/quality_trimmer.cpp
    processing/i_read_processor.cpp
    processing/predicates/min_quality_predicate.cpp
    memory/batch_memory_manager.h
    memory/batch_memory_manager.cpp
)

target_include_directories(fq_legacy_lib 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/gzstream/include
)

target_compile_definitions(fq_legacy_lib PUBLIC SPDLOG_FMT_EXTERNAL)
target_compile_features(fq_legacy_lib PUBLIC cxx_std_20)

target_link_libraries(fq_legacy_lib 
    PUBLIC 
        spdlog::spdlog 
        ZLIB::ZLIB 
        BZip2::BZip2 
        LibLZMA::LibLZMA 
        TBB::tbb 
        mimalloc-static
        fmt::fmt
)

# Create a new main library that will aggregate all components (new modules and legacy code)
add_library(fq_lib INTERFACE)

target_link_libraries(fq_lib 
    INTERFACE 
        fq_config  # This transitively links fq_error and fq_common
        fq_legacy_lib
        fmt::fmt  # Add explicit fmt linking
)

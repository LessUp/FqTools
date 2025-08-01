add_library(fq_fastq INTERFACE)

target_include_directories(fq_fastq
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(fq_fastq INTERFACE cxx_std_20)

# Link dependencies for fastq module
target_link_libraries(fq_fastq
    INTERFACE
        fq_common
        fq_error
        fq_core
        fq_io
)
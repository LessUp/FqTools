add_library(fq_io INTERFACE)

target_include_directories(fq_io
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(fq_io INTERFACE cxx_std_20)

# Link dependencies for io module
target_link_libraries(fq_io
    INTERFACE
        fq_common
        fq_error
        fq_core
)
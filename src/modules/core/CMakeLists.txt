add_library(fq_core INTERFACE)

target_include_directories(fq_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(fq_core INTERFACE cxx_std_20)

# Link dependencies for core module
target_link_libraries(fq_core
    INTERFACE
        fq_common
        fq_error
)
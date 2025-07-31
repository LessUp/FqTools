add_library(fq_error INTERFACE)

target_include_directories(fq_error
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fq_error
    INTERFACE
        fq_common
)

target_compile_features(fq_error INTERFACE cxx_std_20)

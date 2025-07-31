add_library(fq_config STATIC
    config.cpp
    config.h
)

target_include_directories(fq_config
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fq_config
    PUBLIC
        fq_error
)

target_compile_features(fq_config PUBLIC cxx_std_20)

add_library(fq_common STATIC
    common.cpp
    common.h
)

target_include_directories(fq_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fq_common
    PUBLIC
        fmt::fmt
)

target_compile_features(fq_common PUBLIC cxx_std_20)

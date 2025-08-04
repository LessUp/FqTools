add_library(fq_error
    STATIC
        error_codes.h
        error_codes.cpp
        error_context.h
        error_context.cpp
        exception_hierarchy.h
        exception_hierarchy.cpp
        exception_macros.h
        error_recovery.h
        error_recovery.cpp
        error_logger.h
        error_logger.cpp
        error.h
        error.cpp
)

target_include_directories(fq_error
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fq_error
    PUBLIC
        fq_common
        fmt::fmt
        TBB::tbb
)

target_compile_features(fq_error PUBLIC cxx_std_20)

# 设置编译选项
target_compile_options(fq_error
    PRIVATE
        -Wall -Wextra -Wpedantic
)

# 设置 C++ 标准
set_property(TARGET fq_error PROPERTY CXX_STANDARD 20)
set_property(TARGET fq_error PROPERTY CXX_STANDARD_REQUIRED ON)

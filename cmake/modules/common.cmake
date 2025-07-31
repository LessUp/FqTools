# fq.common模块CMakeLists.txt

# 检查编译器是否支持C++20模块
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
    message(WARNING "GCC 13+ is recommended for C++20 modules. Using header-only mode for fq.common module.")
    set(USE_CPP20_MODULES OFF)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    message(WARNING "Clang 16+ is recommended for C++20 modules. Using header-only mode for fq.common module.")
    set(USE_CPP20_MODULES OFF)
else()
    set(USE_CPP20_MODULES ON)
endif()

if(USE_CPP20_MODULES)
    # C++20模块构建
    add_library(fq_common MODULE src/modules/common/common.cppm)
    
    target_compile_features(fq_common PUBLIC cxx_std_20)
    
    # GCC/Clang模块支持
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(fq_common PUBLIC -fmodules-ts)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fq_common PUBLIC -fmodules-ts)
    endif()
    
    target_link_libraries(fq_common PUBLIC
        fmt::fmt
    )
    
    # 设置模块属性
    set_target_properties(fq_common PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
else()
    # 头文件模式构建
    add_library(fq_common INTERFACE)
    
    target_include_directories(fq_common INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/common
    )
    
    target_compile_features(fq_common INTERFACE cxx_std_20)
    
    target_link_libraries(fq_common INTERFACE
        fmt::fmt
    )
endif()

# 安装配置
if(USE_CPP20_MODULES)
    install(TARGETS fq_common
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT Runtime
    )
else()
    install(TARGETS fq_common
        EXPORT FastQToolsTargets
        COMPONENT Development
    )
endif()
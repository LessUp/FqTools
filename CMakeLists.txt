cmake_minimum_required(VERSION 3.20)

project(FastQTools
    LANGUAGES CXX
    VERSION 2.0.0
    DESCRIPTION "A modern toolkit for FASTQ file processing"
    HOMEPAGE_URL "https://github.com/your-org/fastqtools"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Project Configuration ---
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Compiler Version Check ---
# Enforce minimum compiler versions required for C++20 support.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
        message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is too old. "
                            "FastQTools requires GCC 11.0 or newer for C++20 support.")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
        message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is too old. "
                            "FastQTools requires Clang 12.0 or newer for C++20 support.")
    endif()
else()
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER_ID} is not officially supported. "
                    "Your build may fail due to lack of C++20 support.")
endif()


# --- Compiler-specific options ---
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        # Release build optimizations
        add_compile_options(-O3 -march=native)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fno-fat-lto-objects)
        else()
            add_compile_options(-flto=thin)
        endif()
    endif()
endif()

# --- Conan Integration ---
# This assumes you have run `conan install config/dependencies/ --build=missing`
# and the conan_toolchain.cmake file is in the build directory.
if(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    include("${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/config/dependencies/conan_toolchain.cmake")
    include("${CMAKE_SOURCE_DIR}/config/dependencies/conan_toolchain.cmake")
else()
    message(FATAL_ERROR "Conan toolchain not found. Please run 'conan install config/dependencies/ --build=missing' in your build directory.")
endif()

# --- Find Dependencies (handled by Conan) ---
find_package(cxxopts CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(LibLZMA REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)



# --- Code Coverage ---
option(ENABLE_COVERAGE "Enable coverage analysis" OFF)
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

# --- Add Subdirectories ---
# This now handles all library definitions and dependencies internally.
add_subdirectory(src)

# --- Testing ---
option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# --- Create Executable ---
add_executable(FastQTools
    src/cli/main.cpp
)

target_link_libraries(FastQTools
    PRIVATE
    fq_cli # This library contains all command logic and links to all other libs
    spdlog::spdlog
    fmt::fmt
    cxxopts::cxxopts
    ZLIB::ZLIB
    BZip2::BZip2
    LibLZMA::LibLZMA
    nlohmann_json::nlohmann_json
    mimalloc-static
    TBB::tbb
)

target_include_directories(FastQTools 
    PRIVATE 
        src
)

# --- Installation ---
install(TARGETS FastQTools
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Runtime
)

# --- Package Configuration ---
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/FastQTools)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FastQToolsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FastQToolsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FastQToolsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/FastQToolsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/FastQToolsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

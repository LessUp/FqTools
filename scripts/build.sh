#!/bin/bash
# scripts/build.sh
#
# A unified build script for FastQTools with advanced build options.
#
# Usage:
# ./scripts/build.sh [COMPILER] [BUILD_TYPE] [OPTIONS]
#
# Parameters:
#   COMPILER:   'gcc' or 'clang'. Defaults to 'clang'.
#   BUILD_TYPE: 'Debug', 'Release', 'RelWithDebInfo', 'Coverage', 'Sanitize'. Defaults to 'Release'.
#
# Options:
#   --asan:      Enable AddressSanitizer
#   --usan:      Enable UndefinedBehaviorSanitizer
#   --tsan:      Enable ThreadSanitizer
#   --coverage:  Enable code coverage
#   --static:    Enable static analysis
#   --no-lto:    Disable Link Time Optimization
#   --verbose:   Enable verbose output
#   --help:      Show this help message
#
# Examples:
#   ./scripts/build.sh                    # Builds with Clang in Release mode
#   ./scripts/build.sh gcc Debug          # Builds with GCC in Debug mode
#   ./scripts/build.sh clang Sanitize     # Builds with Clang in Sanitize mode
#   ./scripts/build.sh gcc Debug --asan   # Builds with GCC in Debug mode + ASAN
#   ./scripts/build.sh --coverage         # Builds with code coverage
#   ./scripts/build.sh --static           # Builds with static analysis

set -e

# 1. Set Defaults and Process Arguments

# Parse arguments properly
ASAN=false
USAN=false
TSAN=false
COVERAGE=false
STATIC_ANALYSIS=false
LTO=true
VERBOSE=false

# Handle --help first
for arg in "$@"; do
    if [[ "$arg" == "--help" ]]; then
        echo "Usage: $0 [COMPILER] [BUILD_TYPE] [OPTIONS]"
        echo ""
        echo "COMPILER: gcc or clang (default: clang)"
        echo "BUILD_TYPE: Debug, Release, RelWithDebInfo, Coverage, Sanitize (default: Release)"
        echo ""
        echo "Options:"
        echo "  --asan      Enable AddressSanitizer"
        echo "  --usan      Enable UndefinedBehaviorSanitizer"
        echo "  --tsan      Enable ThreadSanitizer"
        echo "  --coverage  Enable code coverage"
        echo "  --static    Enable static analysis"
        echo "  --no-lto    Disable Link Time Optimization"
        echo "  --verbose   Enable verbose output"
        echo "  --help      Show this help message"
        exit 0
    fi
done

# Process positional arguments
if [[ $# -gt 0 && "$1" != "--"* ]]; then
    COMPILER="$1"
    shift
else
    COMPILER="clang"
fi

if [[ $# -gt 0 && "$1" != "--"* ]]; then
    BUILD_TYPE="$1"
    shift
else
    BUILD_TYPE="Release"
fi

BUILD_DIR="build-${COMPILER}-${BUILD_TYPE,,}"

# Process options
while [[ $# -gt 0 ]]; do
    case $1 in
        --asan)
            ASAN=true
            shift
            ;;
        --usan)
            USAN=true
            shift
            ;;
        --tsan)
            TSAN=true
            shift
            ;;
        --coverage)
            COVERAGE=true
            BUILD_TYPE="Coverage"
            shift
            ;;
        --static)
            STATIC_ANALYSIS=true
            shift
            ;;
        --no-lto)
            LTO=false
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help)
            echo "Usage: $0 [COMPILER] [BUILD_TYPE] [OPTIONS]"
            echo ""
            echo "COMPILER: gcc or clang (default: clang)"
            echo "BUILD_TYPE: Debug, Release, RelWithDebInfo, Coverage, Sanitize (default: Release)"
            echo ""
            echo "Options:"
            echo "  --asan      Enable AddressSanitizer"
            echo "  --usan      Enable UndefinedBehaviorSanitizer"
            echo "  --tsan      Enable ThreadSanitizer"
            echo "  --coverage  Enable code coverage"
            echo "  --static    Enable static analysis"
            echo "  --no-lto    Disable Link Time Optimization"
            echo "  --verbose   Enable verbose output"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 2. Validate Arguments
if [[ "$COMPILER" != "gcc" && "$COMPILER" != "clang" ]]; then
    echo "Error: Invalid compiler specified. Choose 'gcc' or 'clang'."
    exit 1
fi

if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "RelWithDebInfo" && "$BUILD_TYPE" != "Coverage" && "$BUILD_TYPE" != "Sanitize" ]]; then
    echo "Error: Invalid build type specified. Choose 'Debug', 'Release', 'RelWithDebInfo', 'Coverage', or 'Sanitize'."
    exit 1
fi

# Check for incompatible sanitizer combinations
if [[ "$ASAN" == true && "$TSAN" == true ]]; then
    echo "Error: Cannot enable both AddressSanitizer and ThreadSanitizer simultaneously"
    exit 1
fi

# 3. Check Prerequisites
if ! command -v conan &> /dev/null; then
    echo "Error: Conan is not installed. Please run scripts/install_dependencies.sh"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed. Please run scripts/install_dependencies.sh"
    exit 1
fi

# Check compiler availability
if [[ "$COMPILER" == "gcc" ]]; then
    if ! command -v g++ &> /dev/null; then
        echo "Error: g++ is not installed. Please run scripts/install_dependencies.sh"
        exit 1
    fi
else
    if ! command -v clang++ &> /dev/null; then
        echo "Error: clang++ is not installed. Please run scripts/install_dependencies.sh"
        exit 1
    fi
fi

# 4. Set Compiler Paths
if [ "$COMPILER" == "gcc" ]; then
    CXX_COMPILER="g++"
else
    CXX_COMPILER="clang++"
fi

echo ">>> Configuring project with ${COMPILER} in ${BUILD_TYPE} mode..."
echo ">>> Build directory: ${BUILD_DIR}"

# 5. Clean previous build if exists
if [ -d "${BUILD_DIR}" ]; then
    echo ">>> Cleaning previous build..."
    rm -rf "${BUILD_DIR}"
fi

# 6. Run Conan Install
CONAN_DIR="${BUILD_DIR}/conan"
echo ">>> Running Conan install..."
if ! conan install config/dependencies/ --output-folder="${CONAN_DIR}" --build=missing -s build_type=${BUILD_TYPE}; then
    echo "Error: Conan install failed"
    exit 1
fi

# 7. Build CMake Options
CMAKE_OPTIONS=()
CMAKE_OPTIONS+=(-DCMAKE_CXX_COMPILER="${CXX_COMPILER}")
CMAKE_OPTIONS+=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
CMAKE_OPTIONS+=(-DCMAKE_TOOLCHAIN_FILE="${CONAN_DIR}/conan_toolchain.cmake")
CMAKE_OPTIONS+=(-G "Ninja")

# Use ccache if available
if command -v ccache &> /dev/null; then
    CMAKE_OPTIONS+=(-DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
fi

# Add sanitizer options
if [[ "$ASAN" == true ]]; then
    CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS="-fsanitize=address -g")
    CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address")
fi

if [[ "$USAN" == true ]]; then
    CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS="${CMAKE_OPTIONS[-1]} -fsanitize=undefined")
    CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS="${CMAKE_OPTIONS[-1]} -fsanitize=undefined")
fi

if [[ "$TSAN" == true ]]; then
    CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS="${CMAKE_OPTIONS[-1]} -fsanitize=thread")
    CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS="${CMAKE_OPTIONS[-1]} -fsanitize=thread")
fi

if [[ "$COVERAGE" == true ]]; then
    CMAKE_OPTIONS+=(-DENABLE_COVERAGE=ON)
    CMAKE_OPTIONS+=(-DCMAKE_CXX_FLAGS="${CMAKE_OPTIONS[-1]} --coverage")
    CMAKE_OPTIONS+=(-DCMAKE_EXE_LINKER_FLAGS="${CMAKE_OPTIONS[-1]} --coverage")
fi

if [[ "$STATIC_ANALYSIS" == true ]]; then
    CMAKE_OPTIONS+=(-DENABLE_STATIC_ANALYSIS=ON)
fi

if [[ "$LTO" == false ]]; then
    CMAKE_OPTIONS+=(-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF)
fi

# 8. Run CMake Configuration
echo ">>> Running CMake configuration..."
if [[ "$VERBOSE" == true ]]; then
    echo "CMake Options: ${CMAKE_OPTIONS[*]}"
fi

if ! cmake -S . -B "${BUILD_DIR}" "${CMAKE_OPTIONS[@]}"; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# 9. Run Build
echo ">>> Building project..."
if ! cmake --build "${BUILD_DIR}"; then
    echo "Error: Build failed"
    exit 1
fi

# 10. Run static analysis if enabled
if [[ "$STATIC_ANALYSIS" == true ]]; then
    echo ">>> Running static analysis..."
    if command -v clang-tidy &> /dev/null; then
        echo "Running clang-tidy..."
        if ! ./scripts/lint.sh -b "${BUILD_DIR}"; then
            echo "Warning: Static analysis found issues"
        fi
    else
        echo "Warning: clang-tidy not found, skipping static analysis"
    fi
fi

# 12. Verify build output
if [ ! -f "${BUILD_DIR}/FastQTools" ]; then
    echo "Error: Executable not found at ${BUILD_DIR}/FastQTools"
    exit 1
fi

echo ">>> Build complete! Executable is at: ${BUILD_DIR}/FastQTools"

# Print build summary
echo ">>> Build Summary:"
echo "  Compiler: ${COMPILER}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Sanitizers: ASAN=$ASAN, USAN=$USAN, TSAN=$TSAN"
echo "  Coverage: $COVERAGE"
echo "  Static Analysis: $STATIC_ANALYSIS"
echo "  LTO: $LTO"

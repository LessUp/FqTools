#!/bin/bash
# scripts/build.sh
#
# A unified build script for FastQTools.
#
# Usage:
# ./scripts/build.sh [COMPILER] [BUILD_TYPE]
#
# Parameters:
#   COMPILER:   'gcc' or 'clang'. Defaults to 'clang'.
#   BUILD_TYPE: 'Debug' or 'Release'. Defaults to 'Release'.
#
# Examples:
#   ./scripts/build.sh                # Builds with Clang in Release mode
#   ./scripts/build.sh gcc Debug      # Builds with GCC in Debug mode
#   ./scripts/build.sh clang Release  # Builds with Clang in Release mode

set -e

# 1. Set Defaults and Process Arguments
COMPILER=${1:-clang}
BUILD_TYPE=${2:-Release}
BUILD_DIR="build-${COMPILER}-${BUILD_TYPE,,}" # e.g., build-clang-release

# 2. Validate Arguments
if [[ "$COMPILER" != "gcc" && "$COMPILER" != "clang" ]]; then
    echo "Error: Invalid compiler specified. Choose 'gcc' or 'clang'."
    exit 1
fi

if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Error: Invalid build type specified. Choose 'Debug' or 'Release'."
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

# 7. Run CMake Configuration
echo ">>> Running CMake configuration..."
if ! cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${CONAN_DIR}/conan_toolchain.cmake" \
    -G "Ninja"; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# 8. Run Build
echo ">>> Building project..."
if ! cmake --build "${BUILD_DIR}"; then
    echo "Error: Build failed"
    exit 1
fi

# 9. Verify build output
if [ ! -f "${BUILD_DIR}/FastQTools" ]; then
    echo "Error: Executable not found at ${BUILD_DIR}/FastQTools"
    exit 1
fi

echo ">>> Build complete! Executable is at: ${BUILD_DIR}/FastQTools"

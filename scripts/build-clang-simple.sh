#!/bin/bash
# scripts/build-clang-simple.sh
#
# 一个简化的Clang构建脚本，专注于解决编译器混用问题

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BUILD_TYPE="Release"
BUILD_DIR="build-clang-release"

echo -e "${BLUE}FastQTools Clang Build Script (Simplified)${NC}"
echo -e "${BLUE}=========================================${NC}"

# Check prerequisites
if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ is not installed${NC}"
    exit 1
fi

if ! command -v conan &> /dev/null; then
    echo -e "${RED}Error: Conan is not installed. Please run scripts/install_dependencies.sh${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed. Please run scripts/install_dependencies.sh${NC}"
    exit 1
fi

# Show Clang version
echo -e "${GREEN}>>> Using Clang: $(clang++ --version | head -n1)${NC}"

# Set environment variables to ensure Clang is used
export CC=clang
export CXX=clang++

# Clean previous build if exists
if [ -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}>>> Cleaning previous build...${NC}"
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"

# Run Conan install with simplified approach
echo -e "${GREEN}>>> Running Conan install...${NC}"
cd "${BUILD_DIR}"
if ! conan install ../config/dependencies/ --build=missing -s build_type="${BUILD_TYPE}"; then
    echo -e "${YELLOW}Warning: Conan install had issues, trying to continue...${NC}"
fi
cd ..

# Run CMake configuration
echo -e "${GREEN}>>> Running CMake configuration...${NC}"
if ! cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -G "Ninja"; then
    echo -e "${RED}Error: CMake configuration failed${NC}"
    exit 1
fi

# Run build
echo -e "${GREEN}>>> Building project...${NC}"
if ! cmake --build "${BUILD_DIR}"; then
    echo -e "${RED}Error: Build failed${NC}"
    exit 1
fi

# Verify build output
if [ ! -f "${BUILD_DIR}/FastQTools" ]; then
    echo -e "${RED}Error: Executable not found at ${BUILD_DIR}/FastQTools${NC}"
    exit 1
fi

echo -e "${GREEN}>>> Build complete! Executable is at: ${BUILD_DIR}/FastQTools${NC}"

# Test the executable if built successfully
echo ""
echo -e "${GREEN}>>> Testing built executable...${NC}"
if "${BUILD_DIR}/FastQTools" --help > /dev/null 2>&1; then
    echo -e "${GREEN}>>> Executable test passed!${NC}"
else
    echo -e "${YELLOW}Warning: Executable test failed, but build completed${NC}"
fi

echo ""
echo -e "${BLUE}Clang Build Summary:${NC}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Build Directory: ${BUILD_DIR}"
echo "  Clang Version: $(clang++ --version | head -n1)"
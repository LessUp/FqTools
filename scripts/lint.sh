#!/bin/bash
# scripts/lint.sh
#
# This script runs clang-tidy and clang-format on the FastQTools codebase.
# It's designed to be used in CI/CD pipelines and for local development.

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_DIR="build-clang-release"
FIX_MODE=false
VERBOSE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -f|--fix)
            FIX_MODE=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -b, --build-dir DIR   Build directory (default: build-clang-release)"
            echo "  -f, --fix            Apply fixes automatically"
            echo "  -v, --verbose        Show verbose output"
            echo "  -h, --help           Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo -e "${GREEN}>>> Running lint checks on FastQTools codebase...${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Warning: Build directory '$BUILD_DIR' does not exist.${NC}"
    echo -e "${YELLOW}Attempting to build project first...${NC}"
    ./scripts/build.sh clang Release
fi

# Check if compile_commands.json exists
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo -e "${RED}Error: compile_commands.json not found in $BUILD_DIR${NC}"
    echo -e "${RED}Please build the project first using ./scripts/build.sh${NC}"
    exit 1
fi

# Check if clang-tidy is available
if ! command -v clang-tidy &> /dev/null; then
    echo -e "${RED}Error: clang-tidy is not installed${NC}"
    echo -e "${RED}Please run ./scripts/install_dependencies.sh${NC}"
    exit 1
fi

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Error: clang-format is not installed${NC}"
    echo -e "${RED}Please run ./scripts/install_dependencies.sh${NC}"
    exit 1
fi

# Find all source files
echo -e "${GREEN}>>> Finding source files...${NC}"
SOURCE_FILES=$(find src/ -name "*.cpp" -o -name "*.hpp" -o -name "*.h")

if [ -z "$SOURCE_FILES" ]; then
    echo -e "${RED}Error: No source files found in src/${NC}"
    exit 1
fi

# Run clang-tidy
echo -e "${GREEN}>>> Running clang-tidy checks...${NC}"
echo -e "${GREEN}Build directory: $BUILD_DIR${NC}"
echo -e "${GREEN}Source files: $(echo "$SOURCE_FILES" | wc -l) files${NC}"

# Run clang-tidy using run-clang-tidy script if available
if command -v run-clang-tidy &> /dev/null; then
    echo -e "${GREEN}Using run-clang-tidy script...${NC}"
    if [ "$FIX_MODE" = true ]; then
        echo -e "${YELLOW}Fix mode enabled - will apply fixes automatically${NC}"
        run-clang-tidy -p "$BUILD_DIR" -fix
    else
        run-clang-tidy -p "$BUILD_DIR"
    fi
else
    echo -e "${YELLOW}run-clang-tidy not found, running individual files...${NC}"
    for file in $SOURCE_FILES; do
        if [ "$FIX_MODE" = true ]; then
            clang-tidy -p "$BUILD_DIR" "$file" -fix
        else
            clang-tidy -p "$BUILD_DIR" "$file"
        fi
    done
fi

# Run clang-format
echo -e "${GREEN}>>> Running clang-format checks...${NC}"
if [ "$FIX_MODE" = true ]; then
    echo -e "${YELLOW}Fix mode enabled - formatting files...${NC}"
    echo "$SOURCE_FILES" | xargs clang-format -i
else
    echo "$SOURCE_FILES" | xargs clang-format --dry-run --Werror
fi

echo -e "${GREEN}>>> All lint checks passed!${NC}"

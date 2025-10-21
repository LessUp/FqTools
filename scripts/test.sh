#!/bin/bash

# FastQTools 测试运行脚本（与 scripts/build.sh 保持一致）
# 使用 ctest --test-dir 在统一的构建目录中运行测试

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认值
COMPILER="clang"         # gcc | clang
BUILD_TYPE="Debug"       # Debug | Release | RelWithDebInfo | Coverage
VERBOSE=false
COVERAGE=false
FILTER=""
CTEST_JOBS=""            # 例如 -j 8
REPEAT=1

print_usage() {
    echo "FastQTools Test Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -c, --compiler NAME    Compiler: gcc | clang (default: clang)"
    echo "  -t, --type TYPE       Build type: Debug|Release|RelWithDebInfo|Coverage (default: Debug)"
    echo "  -v, --verbose          Enable verbose output"
    echo "  -C, --coverage         Generate coverage report"
    echo "  -f, --filter PATTERN   Run only tests matching pattern"
    echo "  -j, --jobs N          Run tests with N parallel jobs (ctest -j N)"
    echo "  -r, --repeat COUNT     Repeat tests COUNT times (default: 1)"
    echo "  -h, --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                     # Run all tests with clang Debug"
    echo "  $0 -c gcc -t Release   # Run tests with gcc Release"
    echo "  $0 -f \"*timer*\"        # Run only timer-related tests"
    echo "  $0 -C                  # Run tests and generate coverage"
    echo "  $0 -r 5                # Run tests 5 times"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--compiler)
            COMPILER="$2"
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -C|--coverage)
            COVERAGE=true
            shift
            ;;
        -f|--filter)
            FILTER="$2"
            shift 2
            ;;
        -j|--jobs)
            CTEST_JOBS="-j $2"
            shift 2
            ;;
        -r|--repeat)
            REPEAT="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

echo -e "${BLUE}FastQTools Test Runner${NC}"
echo -e "${BLUE}=====================${NC}"
echo ""

# 计算构建目录名称
BT_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
BUILD_DIR="build-${COMPILER}-${BT_LOWER}"

# 若不存在则先构建
if [[ ! -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}Build directory not found. Building project first...${NC}"
    if [[ "$COVERAGE" == true || "$BUILD_TYPE" == "Coverage" ]]; then
        ./scripts/build.sh "$COMPILER" Coverage --coverage
        BUILD_DIR="build-${COMPILER}-coverage"
    else
        ./scripts/build.sh "$COMPILER" "$BUILD_TYPE"
    fi
fi

# 构建测试参数
TEST_ARGS=""
if [[ "$VERBOSE" == true ]]; then
    TEST_ARGS="$TEST_ARGS --verbose"
fi

if [[ -n "$FILTER" ]]; then
    TEST_ARGS="$TEST_ARGS -R \"$FILTER\""
fi

# 运行测试
echo -e "${BLUE}Running tests in: $BUILD_DIR${NC}"
echo -e "${BLUE}Test arguments: $TEST_ARGS ${CTEST_JOBS}${NC}"
echo ""

for ((i=1; i<=REPEAT; i++)); do
    if [[ $REPEAT -gt 1 ]]; then
        echo -e "${BLUE}Test run $i of $REPEAT${NC}"
    fi
    
    if eval "ctest --test-dir \"$BUILD_DIR\" $CTEST_JOBS $TEST_ARGS"; then
        echo -e "${GREEN}Tests passed!${NC}"
    else
        echo -e "${RED}Tests failed!${NC}"
        exit 1
    fi
    
    if [[ $i -lt $REPEAT ]]; then
        echo ""
    fi
done

# 生成覆盖率报告
if [[ "$COVERAGE" == true ]]; then
    echo ""
    echo -e "${BLUE}Generating coverage report...${NC}"
    
    if command -v lcov &> /dev/null; then
        # 使用lcov生成覆盖率报告
        lcov --capture --directory "$BUILD_DIR" --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info
        
        if command -v genhtml &> /dev/null; then
            genhtml coverage.info --output-directory coverage_html
            echo -e "${GREEN}Coverage report generated in: coverage_html/index.html${NC}"
        fi
    else
        echo -e "${YELLOW}lcov not found. Coverage report not generated.${NC}"
    fi
fi

echo ""
echo -e "${GREEN}Test execution completed!${NC}"

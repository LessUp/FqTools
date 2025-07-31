#!/bin/bash

# FastQTools 代码格式化脚本
# 使用clang-format统一代码风格

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认值
CHECK_ONLY=false
VERBOSE=false
STYLE_FILE=".clang-format"

print_usage() {
    echo "FastQTools Code Formatter"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -c, --check-only       Only check formatting, don't modify files"
    echo "  -v, --verbose          Enable verbose output"
    echo "  -s, --style FILE       Use specific style file (default: .clang-format)"
    echo "  -h, --help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                     # Format all source files"
    echo "  $0 -c                  # Check formatting without modifying"
    echo "  $0 -v                  # Format with verbose output"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--check-only)
            CHECK_ONLY=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -s|--style)
            STYLE_FILE="$2"
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

echo -e "${BLUE}FastQTools Code Formatter${NC}"
echo -e "${BLUE}========================${NC}"
echo ""

# 检查clang-format是否可用
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Error: clang-format is not installed${NC}"
    echo "Please install clang-format to use this script"
    exit 1
fi

# 检查样式文件是否存在
if [[ ! -f "$STYLE_FILE" ]]; then
    echo -e "${YELLOW}Warning: Style file '$STYLE_FILE' not found${NC}"
    echo "Using default clang-format style"
    STYLE_FILE=""
fi

# 查找需要格式化的文件
echo -e "${BLUE}Finding source files...${NC}"

SOURCE_DIRS=("src" "app" "tests")
FILE_EXTENSIONS=("*.cpp" "*.h" "*.hpp" "*.cppm")

FILES=()
for dir in "${SOURCE_DIRS[@]}"; do
    if [[ -d "$dir" ]]; then
        for ext in "${FILE_EXTENSIONS[@]}"; do
            while IFS= read -r -d '' file; do
                FILES+=("$file")
            done < <(find "$dir" -name "$ext" -type f -print0)
        done
    fi
done

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo -e "${YELLOW}No source files found${NC}"
    exit 0
fi

echo -e "${GREEN}Found ${#FILES[@]} source files${NC}"

# 格式化或检查文件
if [[ "$CHECK_ONLY" == true ]]; then
    echo -e "${BLUE}Checking code formatting...${NC}"
    
    ISSUES_FOUND=false
    for file in "${FILES[@]}"; do
        if [[ "$VERBOSE" == true ]]; then
            echo "Checking: $file"
        fi
        
        if [[ -n "$STYLE_FILE" ]]; then
            if ! clang-format --style=file:"$STYLE_FILE" --dry-run --Werror "$file" &>/dev/null; then
                echo -e "${RED}Formatting issues found in: $file${NC}"
                ISSUES_FOUND=true
            fi
        else
            if ! clang-format --dry-run --Werror "$file" &>/dev/null; then
                echo -e "${RED}Formatting issues found in: $file${NC}"
                ISSUES_FOUND=true
            fi
        fi
    done
    
    if [[ "$ISSUES_FOUND" == true ]]; then
        echo ""
        echo -e "${RED}Code formatting issues found!${NC}"
        echo "Run without --check-only to fix them"
        exit 1
    else
        echo -e "${GREEN}All files are properly formatted!${NC}"
    fi
else
    echo -e "${BLUE}Formatting source files...${NC}"
    
    for file in "${FILES[@]}"; do
        if [[ "$VERBOSE" == true ]]; then
            echo "Formatting: $file"
        fi
        
        if [[ -n "$STYLE_FILE" ]]; then
            clang-format --style=file:"$STYLE_FILE" -i "$file"
        else
            clang-format -i "$file"
        fi
    done
    
    echo -e "${GREEN}Code formatting completed!${NC}"
fi

echo ""
echo -e "${GREEN}Done!${NC}"

#!/bin/bash

# FastQTools 基本统计示例
# 演示如何对单个 FastQ 文件进行统计分析

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认参数
THREADS=${FASTQTOOLS_THREADS:-4}
BATCH_SIZE=${FASTQTOOLS_BATCH_SIZE:-50000}
OUTPUT_DIR="./results"

print_usage() {
    echo "FastQTools 基本统计示例"
    echo ""
    echo "用法: $0 <input.fastq[.gz]> [output_prefix]"
    echo ""
    echo "参数:"
    echo "  input.fastq[.gz]    输入的 FastQ 文件（支持压缩格式）"
    echo "  output_prefix       输出文件前缀（可选，默认使用输入文件名）"
    echo ""
    echo "环境变量:"
    echo "  FASTQTOOLS_THREADS     线程数（默认: 4）"
    echo "  FASTQTOOLS_BATCH_SIZE  批处理大小（默认: 50000）"
    echo ""
    echo "示例:"
    echo "  $0 sample.fastq.gz"
    echo "  $0 data.fq sample_output"
    echo "  FASTQTOOLS_THREADS=8 $0 large_file.fastq.gz"
}

# 检查参数
if [[ $# -lt 1 ]]; then
    print_usage
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_PREFIX="${2:-$(basename "$INPUT_FILE" | sed 's/\.[^.]*$//')}"

# 检查输入文件
if [[ ! -f "$INPUT_FILE" ]]; then
    echo -e "${RED}错误: 输入文件不存在: $INPUT_FILE${NC}"
    exit 1
fi

# 检查 FastQTools 是否可用
if ! command -v fastqtools &> /dev/null; then
    echo -e "${RED}错误: fastqtools 未找到，请确保已正确安装${NC}"
    exit 1
fi

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

echo -e "${BLUE}FastQTools 基本统计分析${NC}"
echo -e "${BLUE}========================${NC}"
echo ""
echo -e "${GREEN}输入文件:${NC} $INPUT_FILE"
echo -e "${GREEN}输出前缀:${NC} $OUTPUT_PREFIX"
echo -e "${GREEN}线程数:${NC} $THREADS"
echo -e "${GREEN}批处理大小:${NC} $BATCH_SIZE"
echo -e "${GREEN}输出目录:${NC} $OUTPUT_DIR"
echo ""

# 获取文件信息
echo -e "${BLUE}文件信息:${NC}"
FILE_SIZE=$(du -h "$INPUT_FILE" | cut -f1)
echo "  文件大小: $FILE_SIZE"

if [[ "$INPUT_FILE" == *.gz ]]; then
    echo "  压缩格式: gzip"
    UNCOMPRESSED_SIZE=$(zcat "$INPUT_FILE" | wc -c | numfmt --to=iec)
    echo "  解压后大小: $UNCOMPRESSED_SIZE"
fi

echo ""

# 执行统计分析
echo -e "${BLUE}开始统计分析...${NC}"
OUTPUT_FILE="$OUTPUT_DIR/${OUTPUT_PREFIX}.stat.txt"

start_time=$(date +%s)

if fastqtools stat \
    -i "$INPUT_FILE" \
    -o "$OUTPUT_FILE" \
    -t "$THREADS" \
    -b "$BATCH_SIZE"; then
    
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    
    echo ""
    echo -e "${GREEN}✅ 统计分析完成！${NC}"
    echo -e "${GREEN}耗时: ${duration} 秒${NC}"
    echo -e "${GREEN}结果文件: $OUTPUT_FILE${NC}"
    
else
    echo -e "${RED}❌ 统计分析失败${NC}"
    exit 1
fi

# 显示结果摘要
echo ""
echo -e "${BLUE}结果摘要:${NC}"
echo -e "${BLUE}========${NC}"

if [[ -f "$OUTPUT_FILE" ]]; then
    # 提取关键统计信息
    if grep -q "Total reads" "$OUTPUT_FILE"; then
        TOTAL_READS=$(grep "Total reads" "$OUTPUT_FILE" | awk '{print $3}')
        echo "  总读取数: $TOTAL_READS"
    fi
    
    if grep -q "Total bases" "$OUTPUT_FILE"; then
        TOTAL_BASES=$(grep "Total bases" "$OUTPUT_FILE" | awk '{print $3}')
        echo "  总碱基数: $TOTAL_BASES"
    fi
    
    if grep -q "Average read length" "$OUTPUT_FILE"; then
        AVG_LENGTH=$(grep "Average read length" "$OUTPUT_FILE" | awk '{print $4}')
        echo "  平均读长: $AVG_LENGTH"
    fi
    
    if grep -q "GC content" "$OUTPUT_FILE"; then
        GC_CONTENT=$(grep "GC content" "$OUTPUT_FILE" | awk '{print $3}')
        echo "  GC含量: $GC_CONTENT"
    fi
    
    if grep -q "Q30 bases" "$OUTPUT_FILE"; then
        Q30_BASES=$(grep "Q30 bases" "$OUTPUT_FILE" | awk '{print $3}')
        echo "  Q30碱基: $Q30_BASES"
    fi
    
    echo ""
    echo -e "${YELLOW}完整结果请查看: $OUTPUT_FILE${NC}"
else
    echo -e "${RED}警告: 结果文件未生成${NC}"
fi

# 生成简单的 HTML 报告（可选）
if command -v python3 &> /dev/null; then
    echo ""
    echo -e "${BLUE}生成 HTML 报告...${NC}"
    
    HTML_FILE="$OUTPUT_DIR/${OUTPUT_PREFIX}.report.html"
    
    python3 -c "
import sys
import os

# 读取统计文件
try:
    with open('$OUTPUT_FILE', 'r') as f:
        content = f.read()
    
    # 生成简单的 HTML 报告
    html = '''<!DOCTYPE html>
<html>
<head>
    <title>FastQ 统计报告 - $OUTPUT_PREFIX</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }
        .stats { background: #f8f9fa; padding: 20px; border-radius: 5px; margin: 20px 0; }
        .highlight { color: #e74c3c; font-weight: bold; }
        pre { background: #2c3e50; color: white; padding: 15px; border-radius: 5px; overflow-x: auto; }
    </style>
</head>
<body>
    <h1 class=\"header\">FastQ 统计报告</h1>
    <div class=\"stats\">
        <h2>文件信息</h2>
        <p><strong>输入文件:</strong> $INPUT_FILE</p>
        <p><strong>分析时间:</strong> $(date)</p>
        <p><strong>处理参数:</strong> 线程数=$THREADS, 批大小=$BATCH_SIZE</p>
    </div>
    <div class=\"stats\">
        <h2>统计结果</h2>
        <pre>''' + content + '''</pre>
    </div>
</body>
</html>'''
    
    with open('$HTML_FILE', 'w') as f:
        f.write(html)
    
    print('HTML 报告已生成: $HTML_FILE')
    
except Exception as e:
    print(f'生成 HTML 报告失败: {e}')
"
fi

echo ""
echo -e "${GREEN}🎉 分析完成！${NC}"
echo ""
echo -e "${YELLOW}下一步建议:${NC}"
echo "1. 查看详细结果: cat $OUTPUT_FILE"
echo "2. 比较多个文件: ./batch_processing.sh *.fastq.gz"
echo "3. 处理双端数据: ./paired_end_analysis.sh read1.fq.gz read2.fq.gz"

if [[ -f "$HTML_FILE" ]]; then
    echo "4. 在浏览器中查看: $HTML_FILE"
fi

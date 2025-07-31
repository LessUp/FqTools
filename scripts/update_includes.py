#!/usr/bin/env python3
"""
脚本用于更新所有源文件中的包含路径，将旧的大写目录名改为小写
"""

import os
import re
import sys
from pathlib import Path

def update_includes_in_file(file_path):
    """更新单个文件中的包含路径"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 更新包含路径的映射
        replacements = {
            r'#include\s+"Common/': '#include "common/',
            r'#include\s+"FastQ/': '#include "fastq/',
            r'#include\s+"FqStatistic/': '#include "statistics/',
            r'#include\s+"Encoder/': '#include "encoder/',
            r'#include\s+"Processing/': '#include "processing/',
            r'#include\s+"Commands/': '#include "commands/',
        }
        
        # 应用所有替换
        for pattern, replacement in replacements.items():
            content = re.sub(pattern, replacement, content)
        
        # 如果内容有变化，写回文件
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Updated: {file_path}")
            return True
        
        return False
        
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False

def main():
    """主函数"""
    # 获取项目根目录
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # 需要处理的目录
    directories = [
        project_root / "src",
        project_root / "app",
        project_root / "tests"
    ]
    
    # 需要处理的文件扩展名
    extensions = ['.cpp', '.h', '.hpp', '.cppm']
    
    updated_files = 0
    total_files = 0
    
    for directory in directories:
        if not directory.exists():
            continue
            
        for ext in extensions:
            for file_path in directory.rglob(f"*{ext}"):
                total_files += 1
                if update_includes_in_file(file_path):
                    updated_files += 1
    
    print(f"\nProcessed {total_files} files, updated {updated_files} files")

if __name__ == "__main__":
    main()

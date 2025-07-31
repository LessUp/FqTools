#!/usr/bin/env python3
"""
C++注释规范化脚本
根据项目注释规范自动为C++文件添加或完善注释
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Optional


class CommentNormalizer:
    """C++注释规范化器"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        # 注释模板
        self.file_comment_template = """/**
 * @file {filename}
 * @brief {brief}
 * @details {details}
 * 
 * @author FastQTools Team
 * @date {date}
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */"""
        
        self.class_comment_template = """/**
 * @brief {brief}
 * @details {details}
 */"""
        
        self.function_comment_template = """/**
 * @brief {brief}
 * @details {details}
 * 
 * @param {param} {param_desc}
 * @return {return_desc}
 */"""
        
        self.struct_comment_template = """/**
 * @brief {brief}
 * @details {details}
 */"""
    
    def normalize_all(self) -> bool:
        """规范化所有需要处理的文件"""
        print("🚀 开始注释规范化...")
        print("=" * 50)
        
        try:
            # 获取需要处理的文件列表
            files_to_process = self._get_files_to_process()
            
            # 处理每个文件
            for file_path in files_to_process:
                print(f"处理文件: {file_path.relative_to(self.project_root)}")
                self._normalize_file(file_path)
            
            print("\n✅ 注释规范化完成！")
            return True
            
        except Exception as e:
            print(f"\n❌ 规范化过程中出现错误: {e}")
            return False
    
    def _get_files_to_process(self) -> List[Path]:
        """获取需要处理的文件列表"""
        # 根据项目需求定义需要处理的文件
        files_to_process = [
            "src/Processing/IReadProcessor.h",
            "src/Core/Core.h",
            "src/cli/commands/ICommand.h",
            "src/cli/commands/FilterCommand.h",
            "src/cli/commands/StatCommand.h",
            "src/modules/config/config.h",
            "src/modules/common/common.h",
            "src/modules/error/error.h"
        ]
        
        paths = []
        for file_rel_path in files_to_process:
            file_path = self.project_root / file_rel_path
            if file_path.exists():
                paths.append(file_path)
        
        return paths
    
    def _normalize_file(self, file_path: Path) -> None:
        """规范化单个文件"""
        try:
            # 读取文件内容
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 备份原文件
            backup_path = file_path.with_suffix(file_path.suffix + '.bak')
            with open(backup_path, 'w', encoding='utf-8') as f:
                f.write(content)
            
            # 规范化文件注释
            normalized_content = self._normalize_content(content, file_path)
            
            # 写入规范化后的内容
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(normalized_content)
            
            print(f"  ✅ 完成: {file_path.relative_to(self.project_root)}")
            
        except Exception as e:
            print(f"  ❌ 错误: {file_path.relative_to(self.project_root)} - {e}")
    
    def _normalize_content(self, content: str, file_path: Path) -> str:
        """规范化文件内容"""
        # 添加文件注释（如果缺失）
        if not self._has_file_comment(content):
            content = self._add_file_comment(content, file_path)
        
        # 添加类注释（如果缺失）
        content = self._add_class_comments(content)
        
        # 添加结构体注释（如果缺失）
        content = self._add_struct_comments(content)
        
        # 添加函数注释（如果缺失）
        content = self._add_function_comments(content)
        
        return content
    
    def _has_file_comment(self, content: str) -> bool:
        """检查是否有文件注释"""
        return "/**" in content and "@file" in content
    
    def _add_file_comment(self, content: str, file_path: Path) -> str:
        """添加文件注释"""
        filename = file_path.name
        brief = f"{filename} 文件"
        details = f"该文件包含 {filename} 的声明和实现"
        import datetime
        date = datetime.datetime.now().strftime("%Y-%m-%d")
        
        file_comment = self.file_comment_template.format(
            filename=filename,
            brief=brief,
            details=details,
            date=date
        )
        
        # 在文件开头添加注释
        return file_comment + "\n\n" + content
    
    def _add_class_comments(self, content: str) -> str:
        """添加类注释"""
        # 查找没有注释的类定义
        class_pattern = r'^\s*class\s+(\w+)'
        lines = content.split('\n')
        new_lines = []
        
        for line in lines:
            if re.match(class_pattern, line) and not self._has_preceding_comment(lines, lines.index(line)):
                class_name = re.match(class_pattern, line).group(1)
                class_comment = self.class_comment_template.format(
                    brief=f"{class_name} 类",
                    details=f"{class_name} 类的详细描述"
                )
                new_lines.append(class_comment)
            new_lines.append(line)
        
        return '\n'.join(new_lines)
    
    def _add_struct_comments(self, content: str) -> str:
        """添加结构体注释"""
        # 查找没有注释的结构体定义
        struct_pattern = r'^\s*struct\s+(\w+)'
        lines = content.split('\n')
        new_lines = []
        
        for line in lines:
            if re.match(struct_pattern, line) and not self._has_preceding_comment(lines, lines.index(line)):
                struct_name = re.match(struct_pattern, line).group(1)
                struct_comment = self.struct_comment_template.format(
                    brief=f"{struct_name} 结构体",
                    details=f"{struct_name} 结构体的详细描述"
                )
                new_lines.append(struct_comment)
            new_lines.append(line)
        
        return '\n'.join(new_lines)
    
    def _add_function_comments(self, content: str) -> str:
        """添加函数注释"""
        # 查找没有注释的函数声明
        function_patterns = [
            r'^\s*auto\s+(\w+)\s*\(.*\)\s*->',
            r'^\s*void\s+(\w+)\s*\(',
            r'^\s*\w+\s+(\w+)\s*\('
        ]
        
        lines = content.split('\n')
        new_lines = []
        
        for line in lines:
            # 检查是否是函数声明且没有前置注释
            is_function = any(re.match(pattern, line) for pattern in function_patterns)
            if is_function and not self._has_preceding_comment(lines, lines.index(line)):
                # 提取函数名
                func_name = None
                for pattern in function_patterns:
                    match = re.match(pattern, line)
                    if match:
                        func_name = match.group(1)
                        break
                
                if func_name:
                    function_comment = self.function_comment_template.format(
                        brief=f"{func_name} 函数",
                        details=f"{func_name} 函数的详细描述",
                        param="参数",
                        param_desc="参数描述",
                        return_desc="返回值描述"
                    )
                    new_lines.append(function_comment)
            new_lines.append(line)
        
        return '\n'.join(new_lines)
    
    def _has_preceding_comment(self, lines: List[str], line_index: int) -> bool:
        """检查指定行前是否有注释"""
        # 检查前几行是否有注释
        for i in range(max(0, line_index - 5), line_index):
            if "/**" in lines[i] or "///" in lines[i]:
                return True
        return False


def main():
    """主函数"""
    project_root = Path(__file__).parent.parent
    normalizer = CommentNormalizer(project_root)
    
    success = normalizer.normalize_all()
    
    if success:
        print("\n🎉 注释规范化完成！")
        print("\n📋 规范化总结:")
        print("- ✅ 为缺失注释的文件添加了文件注释")
        print("- ✅ 为缺失注释的类添加了类注释")
        print("- ✅ 为缺失注释的函数添加了函数注释")
        print("- ✅ 为缺失注释的结构体添加了结构体注释")
        
        print("\n🚀 下一步建议:")
        print("1. 检查生成的注释是否准确")
        print("2. 手动完善注释内容")
        print("3. 运行Doxygen验证注释")
        print("4. 生成API文档")
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
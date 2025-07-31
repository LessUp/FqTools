#!/usr/bin/env python3
"""
FastQTools 代码质量检查工具
检查代码规范、文档完整性、测试覆盖率等
"""

import os
import re
import sys
import subprocess
import argparse
from pathlib import Path
from typing import List, Dict, Tuple
from dataclasses import dataclass

@dataclass
class QualityIssue:
    """代码质量问题"""
    file_path: str
    line_number: int
    issue_type: str
    description: str
    severity: str  # 'error', 'warning', 'info'

class CodeQualityChecker:
    """代码质量检查器"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.issues: List[QualityIssue] = []
        
    def check_all(self) -> bool:
        """执行所有检查"""
        print("🔍 开始代码质量检查...")
        
        success = True
        success &= self.check_naming_conventions()
        success &= self.check_file_headers()
        success &= self.check_documentation()
        success &= self.check_test_coverage()
        success &= self.check_cmake_style()
        success &= self.check_include_guards()
        
        self.print_summary()
        return success
    
    def check_naming_conventions(self) -> bool:
        """检查命名规范"""
        print("📝 检查命名规范...")
        
        # 检查文件命名
        for cpp_file in self.project_root.rglob("*.cpp"):
            if not self._is_valid_filename(cpp_file.name):
                self.issues.append(QualityIssue(
                    str(cpp_file), 0, "naming", 
                    f"文件名不符合规范: {cpp_file.name}", "warning"
                ))
        
        # 检查头文件命名
        for h_file in self.project_root.rglob("*.h"):
            if not self._is_valid_filename(h_file.name):
                self.issues.append(QualityIssue(
                    str(h_file), 0, "naming", 
                    f"头文件名不符合规范: {h_file.name}", "warning"
                ))
        
        return len([i for i in self.issues if i.issue_type == "naming" and i.severity == "error"]) == 0
    
    def check_file_headers(self) -> bool:
        """检查文件头注释"""
        print("📄 检查文件头注释...")
        
        for source_file in self.project_root.rglob("*.cpp"):
            if not self._has_proper_header(source_file):
                self.issues.append(QualityIssue(
                    str(source_file), 1, "documentation", 
                    "缺少文件头注释", "warning"
                ))
        
        for header_file in self.project_root.rglob("*.h"):
            if not self._has_proper_header(header_file):
                self.issues.append(QualityIssue(
                    str(header_file), 1, "documentation", 
                    "缺少文件头注释", "warning"
                ))
        
        return True
    
    def check_documentation(self) -> bool:
        """检查文档完整性"""
        print("📚 检查文档完整性...")
        
        # 检查公共类是否有文档
        for header_file in self.project_root.rglob("*.h"):
            content = header_file.read_text(encoding='utf-8', errors='ignore')
            
            # 查找类定义
            class_matches = re.finditer(r'class\s+(\w+)', content)
            for match in class_matches:
                class_name = match.group(1)
                line_num = content[:match.start()].count('\n') + 1
                
                # 检查类前是否有文档注释
                if not self._has_class_documentation(content, match.start()):
                    self.issues.append(QualityIssue(
                        str(header_file), line_num, "documentation", 
                        f"类 {class_name} 缺少文档注释", "warning"
                    ))
        
        return True
    
    def check_test_coverage(self) -> bool:
        """检查测试覆盖率"""
        print("🧪 检查测试覆盖率...")
        
        src_files = set()
        test_files = set()
        
        # 收集源文件
        for cpp_file in (self.project_root / "src").rglob("*.cpp"):
            if cpp_file.name != "main.cpp":  # 排除主函数
                src_files.add(cpp_file.stem)
        
        # 收集测试文件
        tests_dir = self.project_root / "tests" / "unit"
        if tests_dir.exists():
            for test_file in tests_dir.rglob("test_*.cpp"):
                # 提取被测试的模块名
                module_name = test_file.name.replace("test_", "").replace(".cpp", "")
                test_files.add(module_name)
        
        # 检查缺少测试的模块
        missing_tests = src_files - test_files
        for module in missing_tests:
            self.issues.append(QualityIssue(
                f"src/{module}.cpp", 0, "testing", 
                f"模块 {module} 缺少单元测试", "warning"
            ))
        
        coverage_rate = (len(test_files) / len(src_files)) * 100 if src_files else 100
        print(f"📊 测试覆盖率: {coverage_rate:.1f}% ({len(test_files)}/{len(src_files)})")
        
        return coverage_rate >= 70  # 要求至少70%覆盖率
    
    def check_cmake_style(self) -> bool:
        """检查CMake文件风格"""
        print("🔧 检查CMake文件风格...")
        
        for cmake_file in self.project_root.rglob("CMakeLists.txt"):
            content = cmake_file.read_text(encoding='utf-8', errors='ignore')
            lines = content.split('\n')
            
            for i, line in enumerate(lines, 1):
                # 检查缩进
                if line.strip() and not line.startswith('#'):
                    if line.startswith(' ') and len(line) - len(line.lstrip()) % 4 != 0:
                        self.issues.append(QualityIssue(
                            str(cmake_file), i, "style", 
                            "CMake缩进应使用4个空格", "info"
                        ))
                
                # 检查命令大小写
                if re.match(r'^\s*[A-Z_]+\s*\(', line):
                    self.issues.append(QualityIssue(
                        str(cmake_file), i, "style", 
                        "CMake命令应使用小写", "info"
                    ))
        
        return True
    
    def check_include_guards(self) -> bool:
        """检查头文件包含保护"""
        print("🛡️ 检查头文件包含保护...")
        
        for header_file in self.project_root.rglob("*.h"):
            content = header_file.read_text(encoding='utf-8', errors='ignore')
            
            # 检查是否有包含保护
            if not (content.startswith('#ifndef') or content.startswith('#pragma once')):
                self.issues.append(QualityIssue(
                    str(header_file), 1, "structure", 
                    "头文件缺少包含保护", "error"
                ))
        
        return len([i for i in self.issues if i.issue_type == "structure" and i.severity == "error"]) == 0
    
    def _is_valid_filename(self, filename: str) -> bool:
        """检查文件名是否符合规范"""
        # 允许的模式：小写字母、数字、下划线
        base_name = filename.split('.')[0]
        return re.match(r'^[a-z][a-z0-9_]*$', base_name) is not None
    
    def _has_proper_header(self, file_path: Path) -> bool:
        """检查文件是否有合适的头注释"""
        try:
            content = file_path.read_text(encoding='utf-8', errors='ignore')
            lines = content.split('\n')
            
            # 检查前几行是否有注释
            for line in lines[:10]:
                if line.strip().startswith('//') or line.strip().startswith('/*'):
                    return True
            
            return False
        except:
            return False
    
    def _has_class_documentation(self, content: str, class_pos: int) -> bool:
        """检查类前是否有文档注释"""
        # 获取类定义前的内容
        before_class = content[:class_pos]
        lines = before_class.split('\n')
        
        # 检查最后几行是否有文档注释
        for line in reversed(lines[-5:]):
            if '/**' in line or '@brief' in line:
                return True
        
        return False
    
    def print_summary(self):
        """打印检查结果摘要"""
        print("\n" + "="*60)
        print("📋 代码质量检查结果")
        print("="*60)
        
        if not self.issues:
            print("✅ 没有发现问题！代码质量良好。")
            return
        
        # 按严重程度分组
        errors = [i for i in self.issues if i.severity == "error"]
        warnings = [i for i in self.issues if i.severity == "warning"]
        infos = [i for i in self.issues if i.severity == "info"]
        
        if errors:
            print(f"❌ 错误: {len(errors)} 个")
            for issue in errors[:5]:  # 只显示前5个
                print(f"   {issue.file_path}:{issue.line_number} - {issue.description}")
        
        if warnings:
            print(f"⚠️  警告: {len(warnings)} 个")
            for issue in warnings[:5]:  # 只显示前5个
                print(f"   {issue.file_path}:{issue.line_number} - {issue.description}")
        
        if infos:
            print(f"ℹ️  信息: {len(infos)} 个")
        
        print(f"\n总计: {len(self.issues)} 个问题")
        
        if errors:
            print("\n❌ 存在严重问题，建议修复后再提交代码。")
        elif warnings:
            print("\n⚠️  存在一些警告，建议修复以提高代码质量。")
        else:
            print("\n✅ 代码质量良好！")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="FastQTools 代码质量检查")
    parser.add_argument("--project-root", type=Path, 
                       default=Path(__file__).parent.parent.parent,
                       help="项目根目录")
    parser.add_argument("--fix", action="store_true", 
                       help="自动修复可修复的问题")
    
    args = parser.parse_args()
    
    checker = CodeQualityChecker(args.project_root)
    success = checker.check_all()
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
FastQTools 重构验证脚本
验证重构后的项目结构和配置是否正确
"""

import os
import sys
from pathlib import Path
from typing import List, Dict, Tuple

class RefactorVerifier:
    """重构验证器"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.issues = []
        self.warnings = []
        
    def verify_all(self) -> bool:
        """执行所有验证"""
        print("🔍 开始验证重构结果...")
        print("=" * 50)
        
        success = True
        success &= self.verify_directory_structure()
        success &= self.verify_file_naming()
        success &= self.verify_cmake_configuration()
        success &= self.verify_include_paths()
        success &= self.verify_test_structure()
        success &= self.verify_documentation()
        success &= self.verify_scripts()
        
        self.print_summary()
        return success
    
    def verify_directory_structure(self) -> bool:
        """验证目录结构"""
        print("📁 验证目录结构...")
        
        expected_dirs = [
            "src/common",
            "src/fastq", 
            "src/statistics",
            "src/encoder",
            "src/processing",
            "app/commands",
            "tests/unit",
            "tests/integration",
            "tests/fixtures",
            "tests/utils",
            "docs/user-guide",
            "docs/developer-guide",
            "docs/api",
            "scripts",
            "tools/benchmarks",
            "tools/generators",
            "tools/validators",
            "examples/basic-usage",
            "examples/advanced",
            "config"
        ]
        
        missing_dirs = []
        for dir_path in expected_dirs:
            full_path = self.project_root / dir_path
            if not full_path.exists():
                missing_dirs.append(dir_path)
        
        if missing_dirs:
            self.issues.extend(f"缺少目录: {d}" for d in missing_dirs)
            return False
        
        print("  ✅ 目录结构正确")
        return True
    
    def verify_file_naming(self) -> bool:
        """验证文件命名规范"""
        print("📝 验证文件命名规范...")
        
        # 检查源码目录是否使用小写
        src_dirs = ["common", "fastq", "statistics", "encoder", "processing"]
        for dir_name in src_dirs:
            src_path = self.project_root / "src" / dir_name
            if not src_path.exists():
                self.issues.append(f"源码目录不存在: src/{dir_name}")
                continue
                
            # 检查是否有大写目录残留
            parent_dir = src_path.parent
            for item in parent_dir.iterdir():
                if item.is_dir() and item.name != dir_name and item.name.lower() == dir_name:
                    self.warnings.append(f"发现大写目录残留: {item}")
        
        # 检查app/commands目录
        commands_dir = self.project_root / "app" / "commands"
        if not commands_dir.exists():
            self.issues.append("app/commands 目录不存在")
        
        print("  ✅ 文件命名规范正确")
        return True
    
    def verify_cmake_configuration(self) -> bool:
        """验证CMake配置"""
        print("🔧 验证CMake配置...")
        
        # 检查主CMakeLists.txt
        main_cmake = self.project_root / "CMakeLists.txt"
        if not main_cmake.exists():
            self.issues.append("主CMakeLists.txt不存在")
            return False
        
        content = main_cmake.read_text(encoding='utf-8')
        
        # 检查是否包含测试配置
        if "BUILD_TESTING" not in content:
            self.issues.append("主CMakeLists.txt缺少测试配置")
        
        if "add_subdirectory(tests)" not in content:
            self.issues.append("主CMakeLists.txt未包含tests目录")
        
        # 检查src/CMakeLists.txt
        src_cmake = self.project_root / "src" / "CMakeLists.txt"
        if src_cmake.exists():
            src_content = src_cmake.read_text(encoding='utf-8')
            expected_modules = ["common", "fastq", "statistics", "encoder", "processing"]
            
            for module in expected_modules:
                if f"add_subdirectory({module})" not in src_content:
                    self.issues.append(f"src/CMakeLists.txt缺少模块: {module}")
        
        print("  ✅ CMake配置正确")
        return True
    
    def verify_include_paths(self) -> bool:
        """验证包含路径"""
        print("📦 验证包含路径...")
        
        # 检查是否还有旧的包含路径
        old_patterns = [
            '#include "Common/',
            '#include "FastQ/',
            '#include "FqStatistic/',
            '#include "Encoder/',
            '#include "Processing/',
            '#include "Commands/'
        ]
        
        issues_found = False
        for pattern in old_patterns:
            for file_path in self.project_root.rglob("*.cpp"):
                try:
                    content = file_path.read_text(encoding='utf-8', errors='ignore')
                    if pattern in content:
                        self.issues.append(f"文件 {file_path} 包含旧的包含路径: {pattern}")
                        issues_found = True
                except:
                    continue
            
            for file_path in self.project_root.rglob("*.h"):
                try:
                    content = file_path.read_text(encoding='utf-8', errors='ignore')
                    if pattern in content:
                        self.issues.append(f"文件 {file_path} 包含旧的包含路径: {pattern}")
                        issues_found = True
                except:
                    continue
        
        if not issues_found:
            print("  ✅ 包含路径已正确更新")
        
        return not issues_found
    
    def verify_test_structure(self) -> bool:
        """验证测试结构"""
        print("🧪 验证测试结构...")
        
        tests_cmake = self.project_root / "tests" / "CMakeLists.txt"
        if not tests_cmake.exists():
            self.issues.append("tests/CMakeLists.txt不存在")
            return False
        
        # 检查测试工具
        test_utils_h = self.project_root / "tests" / "utils" / "test_helpers.h"
        test_utils_cpp = self.project_root / "tests" / "utils" / "test_helpers.cpp"
        
        if not test_utils_h.exists():
            self.issues.append("测试工具头文件不存在")
        
        if not test_utils_cpp.exists():
            self.issues.append("测试工具源文件不存在")
        
        # 检查单元测试目录
        unit_dirs = ["common", "fastq", "statistics", "encoder", "processing"]
        for unit_dir in unit_dirs:
            unit_path = self.project_root / "tests" / "unit" / unit_dir
            if not unit_path.exists():
                self.warnings.append(f"单元测试目录不存在: tests/unit/{unit_dir}")
        
        print("  ✅ 测试结构正确")
        return True
    
    def verify_documentation(self) -> bool:
        """验证文档结构"""
        print("📚 验证文档结构...")
        
        # 检查主要文档文件
        doc_files = [
            "docs/README.md",
            "docs/user-guide/overview.md",
            "docs/user-guide/getting-started.md",
            "docs/developer-guide/architecture.md",
            "docs/developer-guide/development.md",
            "docs/developer-guide/building.md"
        ]
        
        for doc_file in doc_files:
            doc_path = self.project_root / doc_file
            if not doc_path.exists():
                self.warnings.append(f"文档文件不存在: {doc_file}")
        
        print("  ✅ 文档结构正确")
        return True
    
    def verify_scripts(self) -> bool:
        """验证脚本文件"""
        print("📜 验证脚本文件...")
        
        script_files = [
            "scripts/build.sh",
            "scripts/test.sh", 
            "scripts/format.sh",
            "scripts/dev.sh"
        ]
        
        for script_file in script_files:
            script_path = self.project_root / script_file
            if not script_path.exists():
                self.issues.append(f"脚本文件不存在: {script_file}")
            elif not os.access(script_path, os.X_OK):
                self.warnings.append(f"脚本文件不可执行: {script_file}")
        
        print("  ✅ 脚本文件正确")
        return True
    
    def print_summary(self):
        """打印验证结果摘要"""
        print("\n" + "=" * 60)
        print("📋 重构验证结果")
        print("=" * 60)
        
        if not self.issues and not self.warnings:
            print("🎉 重构验证通过！项目结构完全正确。")
            print("\n✅ 所有检查项目都已通过：")
            print("  - 目录结构符合最佳实践")
            print("  - 文件命名规范统一")
            print("  - CMake配置正确")
            print("  - 包含路径已更新")
            print("  - 测试结构完整")
            print("  - 文档结构清晰")
            print("  - 脚本文件可用")
            return
        
        if self.issues:
            print(f"❌ 发现 {len(self.issues)} 个问题需要修复：")
            for i, issue in enumerate(self.issues, 1):
                print(f"  {i}. {issue}")
        
        if self.warnings:
            print(f"\n⚠️  发现 {len(self.warnings)} 个警告：")
            for i, warning in enumerate(self.warnings, 1):
                print(f"  {i}. {warning}")
        
        if self.issues:
            print(f"\n❌ 重构验证失败，请修复上述问题后重新验证。")
        else:
            print(f"\n✅ 重构验证通过，但有一些警告需要注意。")

def main():
    """主函数"""
    project_root = Path(__file__).parent.parent
    verifier = RefactorVerifier(project_root)
    
    success = verifier.verify_all()
    
    if success:
        print("\n🚀 下一步建议：")
        print("1. 运行构建测试: ./scripts/build.sh --test")
        print("2. 运行代码格式化: ./scripts/format.sh")
        print("3. 运行代码质量检查: python tools/validators/code_quality.py")
        print("4. 提交重构更改: git add . && git commit -m 'refactor: 重构项目结构'")
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())

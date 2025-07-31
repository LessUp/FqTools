#!/usr/bin/env python3
"""
FastQTools 目录结构优化脚本
按照设计方案重组项目目录结构
"""

import os
import shutil
import sys
from pathlib import Path
from typing import Dict, List, Tuple

class StructureOptimizer:
    """目录结构优化器"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.backup_created = False
        
    def optimize_all(self) -> bool:
        """执行完整的结构优化"""
        print("🚀 开始目录结构优化...")
        print("=" * 50)
        
        try:
            # 阶段2：文件重组
            self.reorganize_documentation()
            self.move_dockerfile()
            
            # 阶段3：配置更新
            self.update_cmake_configs()
            
            # 阶段5：清理
            self.cleanup_obsolete_files()
            self.update_gitignore()
            
            print("\n✅ 目录结构优化完成！")
            return True
            
        except Exception as e:
            print(f"\n❌ 优化过程中出现错误: {e}")
            return False
    
    def reorganize_documentation(self):
        """重组文档结构"""
        print("📚 重组文档结构...")
        
        docs_dir = self.project_root / "docs"
        
        # 移动现有文档到新结构
        moves = [
            # 用户文档
            ("user-guide/overview.md", "user/overview.md"),
            ("user-guide/getting-started.md", "user/installation.md"),
            
            # 开发者文档  
            ("developer-guide/architecture.md", "dev/architecture.md"),
            ("developer-guide/development.md", "dev/contributing.md"),
            ("developer-guide/building.md", "dev/building.md"),
            
            # API文档
            ("api", "dev/api"),
        ]
        
        for src, dst in moves:
            src_path = docs_dir / src
            dst_path = docs_dir / dst
            
            if src_path.exists():
                # 确保目标目录存在
                dst_path.parent.mkdir(parents=True, exist_ok=True)
                
                if src_path.is_dir():
                    if dst_path.exists():
                        shutil.rmtree(dst_path)
                    shutil.move(str(src_path), str(dst_path))
                else:
                    shutil.move(str(src_path), str(dst_path))
                
                print(f"  ✅ 移动: {src} → {dst}")
        
        # 清理空目录
        for old_dir in ["user-guide", "developer-guide"]:
            old_path = docs_dir / old_dir
            if old_path.exists() and not any(old_path.iterdir()):
                old_path.rmdir()
                print(f"  🗑️ 删除空目录: {old_dir}")
    
    def move_dockerfile(self):
        """移动Dockerfile到config目录"""
        print("🐳 移动Docker配置...")
        
        dockerfile = self.project_root / "Dockerfile"
        config_dir = self.project_root / "config"
        
        if dockerfile.exists():
            shutil.move(str(dockerfile), str(config_dir / "Dockerfile"))
            print("  ✅ 移动: Dockerfile → config/Dockerfile")
    
    def update_cmake_configs(self):
        """更新CMake配置"""
        print("🔧 更新CMake配置...")
        
        # 确保CMake配置在根目录（已经在正确位置）
        root_cmake = self.project_root / "CMakeLists.txt"
        root_presets = self.project_root / "CMakePresets.json"
        
        if root_cmake.exists() and root_presets.exists():
            print("  ✅ CMake配置已在正确位置")
        
        # 更新构建脚本中的配置路径引用
        self.update_build_scripts()
    
    def update_build_scripts(self):
        """更新构建脚本中的路径引用"""
        print("📜 更新构建脚本...")
        
        build_script = self.project_root / "scripts" / "build.sh"
        if build_script.exists():
            content = build_script.read_text(encoding='utf-8')
            
            # 更新conan install路径
            updated_content = content.replace(
                'conan install ../../config',
                'conan install ../config'
            )
            
            if content != updated_content:
                build_script.write_text(updated_content, encoding='utf-8')
                print("  ✅ 更新build.sh中的配置路径")
    
    def cleanup_obsolete_files(self):
        """清理废弃文件"""
        print("🗑️ 清理废弃文件...")
        
        obsolete_files = [
            "CMakeUserPresets.json",  # 用户特定配置，不应提交
            "REFACTOR_SUMMARY.md",    # 临时文档
            "CLAUDE.md",              # 临时文档
        ]
        
        for file_name in obsolete_files:
            file_path = self.project_root / file_name
            if file_path.exists():
                file_path.unlink()
                print(f"  🗑️ 删除: {file_name}")
    
    def update_gitignore(self):
        """更新.gitignore规则"""
        print("📝 更新.gitignore...")
        
        gitignore_path = self.project_root / ".gitignore"
        
        # 新的忽略规则
        new_rules = [
            "",
            "# Build directories",
            "build/",
            "cmake-build-*/",
            "",
            "# IDE files", 
            ".idea/",
            ".vscode/",
            "*.swp",
            "*.swo",
            "",
            "# User-specific CMake presets",
            "CMakeUserPresets.json",
            "",
            "# Temporary files",
            "*.tmp",
            "*.log",
            "",
            "# OS specific",
            ".DS_Store",
            "Thumbs.db",
        ]
        
        if gitignore_path.exists():
            content = gitignore_path.read_text(encoding='utf-8')
        else:
            content = ""
        
        # 添加新规则（如果不存在）
        for rule in new_rules:
            if rule and rule not in content:
                content += f"\n{rule}"
        
        gitignore_path.write_text(content, encoding='utf-8')
        print("  ✅ 更新.gitignore规则")
    
    def create_missing_files(self):
        """创建缺失的重要文件"""
        print("📄 创建缺失文件...")
        
        files_to_create = [
            ("LICENSE", self.get_license_content()),
            ("CHANGELOG.md", self.get_changelog_content()),
            ("third_party/README.md", self.get_third_party_readme()),
            ("docs/user/usage.md", self.get_usage_guide()),
            ("docs/dev/coding-standards.md", self.get_coding_standards()),
            ("docs/design/requirements.md", self.get_requirements_doc()),
        ]
        
        for file_path, content in files_to_create:
            full_path = self.project_root / file_path
            if not full_path.exists():
                full_path.parent.mkdir(parents=True, exist_ok=True)
                full_path.write_text(content, encoding='utf-8')
                print(f"  ✅ 创建: {file_path}")
    
    def get_license_content(self) -> str:
        """获取许可证内容"""
        return """MIT License

Copyright (c) 2024 FastQTools

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""
    
    def get_changelog_content(self) -> str:
        """获取变更日志内容"""
        return """# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- 重构项目目录结构
- 建立完整的测试框架
- 添加开发工具和脚本

### Changed
- 统一目录命名规范
- 重组文档结构
- 优化构建系统

## [2.0.0] - 2024-XX-XX

### Added
- 初始版本发布
"""
    
    def get_third_party_readme(self) -> str:
        """获取第三方依赖说明"""
        return """# Third Party Dependencies

This directory contains third-party dependencies that are included directly in the project.

## Current Dependencies

Currently, all dependencies are managed through Conan and vcpkg package managers.
No direct third-party code is included in this directory.

## Adding New Dependencies

When adding new third-party dependencies:

1. Prefer package managers (Conan/vcpkg) over direct inclusion
2. If direct inclusion is necessary, add the dependency here
3. Update this README with dependency information
4. Ensure proper licensing compliance

## License Compliance

All third-party dependencies must be compatible with the project's MIT license.
"""
    
    def get_usage_guide(self) -> str:
        """获取使用指南"""
        return """# Usage Guide

## Basic Usage

### FastQ File Statistics

```bash
fastqtools stat -i input.fastq.gz -o output.txt
```

### Batch Processing

```bash
fastqtools stat -i *.fastq.gz -o batch_results/
```

## Advanced Usage

For advanced usage examples, see the [examples directory](../../examples/).

## Command Reference

For detailed command reference, run:

```bash
fastqtools --help
fastqtools stat --help
```
"""
    
    def get_coding_standards(self) -> str:
        """获取编码规范"""
        return """# Coding Standards

## C++ Standards

- Use C++20 features
- Follow Google C++ Style Guide
- Use clang-format for code formatting

## Naming Conventions

- Classes: PascalCase
- Functions: camelCase
- Variables: snake_case
- Constants: UPPER_SNAKE_CASE
- Files: snake_case

## Documentation

- Use Doxygen comments for public APIs
- Include examples in documentation
- Keep README files up to date

## Testing

- Write unit tests for all new features
- Maintain test coverage above 80%
- Use descriptive test names
"""
    
    def get_requirements_doc(self) -> str:
        """获取需求文档"""
        return """# Requirements Document

## Functional Requirements

### Core Features
- FastQ file reading and writing
- Statistical analysis of sequence data
- Batch processing capabilities
- Multi-threading support

### Performance Requirements
- Process files up to 100GB
- Support for compressed formats
- Memory-efficient processing

## Non-Functional Requirements

### Usability
- Command-line interface
- Clear error messages
- Comprehensive documentation

### Reliability
- Robust error handling
- Data integrity validation
- Graceful failure recovery

### Performance
- Multi-core utilization
- Streaming processing for large files
- Configurable memory usage
"""

def main():
    """主函数"""
    project_root = Path(__file__).parent.parent
    optimizer = StructureOptimizer(project_root)
    
    success = optimizer.optimize_all()
    
    if success:
        print("\n🎉 目录结构优化完成！")
        print("\n📋 优化总结:")
        print("- ✅ 重组文档结构")
        print("- ✅ 移动配置文件")
        print("- ✅ 更新构建脚本")
        print("- ✅ 清理废弃文件")
        print("- ✅ 更新.gitignore")
        
        print("\n🚀 下一步建议:")
        print("1. 运行构建测试: ./scripts/build.sh")
        print("2. 验证文档结构: ls -la docs/")
        print("3. 检查配置文件: ls -la config/")
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())

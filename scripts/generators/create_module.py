#!/usr/bin/env python3
"""
FastQTools 模块生成器
用于快速创建新的模块结构
"""

import os
import sys
import argparse
from pathlib import Path
from typing import Dict, List

class ModuleGenerator:
    """模块生成器类"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.src_dir = project_root / "src"
        self.tests_dir = project_root / "tests" / "unit"
        
    def create_module(self, module_name: str, module_type: str = "library") -> bool:
        """创建新模块"""
        try:
            # 创建源码目录
            module_dir = self.src_dir / module_name
            module_dir.mkdir(exist_ok=True)
            
            # 创建测试目录
            test_dir = self.tests_dir / module_name
            test_dir.mkdir(exist_ok=True)
            
            # 生成文件
            self._create_cmake_file(module_dir, module_name, module_type)
            self._create_header_file(module_dir, module_name)
            self._create_source_file(module_dir, module_name)
            self._create_test_file(test_dir, module_name)
            
            # 更新主CMakeLists.txt
            self._update_main_cmake(module_name)
            
            print(f"✅ 模块 '{module_name}' 创建成功!")
            print(f"📁 源码目录: {module_dir}")
            print(f"🧪 测试目录: {test_dir}")
            
            return True
            
        except Exception as e:
            print(f"❌ 创建模块失败: {e}")
            return False
    
    def _create_cmake_file(self, module_dir: Path, module_name: str, module_type: str):
        """创建CMakeLists.txt文件"""
        content = f"""# {module_name.title()} 模块 CMakeLists.txt

# 设置模块源文件
set({module_name.upper()}_SOURCES
    {module_name}.cpp
)

set({module_name.upper()}_HEADERS
    {module_name}.h
)

# 创建 {module_name} 库
add_library(fastq_{module_name} STATIC ${{{module_name.upper()}_SOURCES}})

# 设置目标属性
set_target_properties(fastq_{module_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 包含目录
target_include_directories(fastq_{module_name}
    PUBLIC
        ${{CMAKE_CURRENT_SOURCE_DIR}}
        ${{CMAKE_SOURCE_DIR}}/src
    PRIVATE
        ${{CMAKE_CURRENT_SOURCE_DIR}}
)

# 链接依赖库
target_link_libraries(fastq_{module_name}
    PUBLIC
        fastq_common
        spdlog::spdlog
    PRIVATE
        ${{CMAKE_THREAD_LIBS_INIT}}
)

# 安装配置
install(TARGETS fastq_{module_name}
    LIBRARY DESTINATION ${{CMAKE_INSTALL_LIBDIR}}
    ARCHIVE DESTINATION ${{CMAKE_INSTALL_LIBDIR}}
    RUNTIME DESTINATION ${{CMAKE_INSTALL_BINDIR}}
)

install(FILES ${{{module_name.upper()}_HEADERS}}
    DESTINATION ${{CMAKE_INSTALL_INCLUDEDIR}}/fastqtools/{module_name}
)
"""
        
        cmake_file = module_dir / "CMakeLists.txt"
        cmake_file.write_text(content, encoding='utf-8')
    
    def _create_header_file(self, module_dir: Path, module_name: str):
        """创建头文件"""
        guard_name = f"FASTQTOOLS_{module_name.upper()}_H"
        namespace_name = f"fq::{module_name}"
        
        content = f"""#ifndef {guard_name}
#define {guard_name}

#include <string>
#include <memory>

namespace {namespace_name} {{

/**
 * @brief {module_name.title()} 模块主类
 */
class {module_name.title()} {{
public:
    /**
     * @brief 构造函数
     */
    {module_name.title()}();
    
    /**
     * @brief 析构函数
     */
    virtual ~{module_name.title()}() = default;
    
    /**
     * @brief 初始化模块
     * @return 是否成功
     */
    bool initialize();
    
    /**
     * @brief 获取模块名称
     * @return 模块名称
     */
    std::string getName() const;
    
    /**
     * @brief 获取模块版本
     * @return 版本字符串
     */
    std::string getVersion() const;

private:
    bool initialized_;
    std::string name_;
}};

}} // namespace {namespace_name}

#endif // {guard_name}
"""
        
        header_file = module_dir / f"{module_name}.h"
        header_file.write_text(content, encoding='utf-8')
    
    def _create_source_file(self, module_dir: Path, module_name: str):
        """创建源文件"""
        namespace_name = f"fq::{module_name}"
        
        content = f"""#include "{module_name}.h"
#include "spdlog/spdlog.h"

namespace {namespace_name} {{

{module_name.title()}::{module_name.title()}() 
    : initialized_(false), name_("{module_name}") {{
    spdlog::debug("Creating {{}} module", name_);
}}

bool {module_name.title()}::initialize() {{
    if (initialized_) {{
        spdlog::warn("{{}} module already initialized", name_);
        return true;
    }}
    
    spdlog::info("Initializing {{}} module", name_);
    
    // TODO: 添加初始化逻辑
    
    initialized_ = true;
    spdlog::info("{{}} module initialized successfully", name_);
    return true;
}}

std::string {module_name.title()}::getName() const {{
    return name_;
}}

std::string {module_name.title()}::getVersion() const {{
    return "1.0.0";
}}

}} // namespace {namespace_name}
"""
        
        source_file = module_dir / f"{module_name}.cpp"
        source_file.write_text(content, encoding='utf-8')
    
    def _create_test_file(self, test_dir: Path, module_name: str):
        """创建测试文件"""
        content = f"""#include <gtest/gtest.h>
#include "test_helpers.h"
#include "{module_name}.h"

namespace fq::test {{

class {module_name.title()}Test : public FastQToolsTest {{
protected:
    void SetUp() override {{
        FastQToolsTest::SetUp();
        module_ = std::make_unique<fq::{module_name}::{module_name.title()}>();
    }}
    
    void TearDown() override {{
        module_.reset();
        FastQToolsTest::TearDown();
    }}
    
    std::unique_ptr<fq::{module_name}::{module_name.title()}> module_;
}};

TEST_F({module_name.title()}Test, Construction) {{
    EXPECT_NE(module_, nullptr);
    EXPECT_EQ(module_->getName(), "{module_name}");
    EXPECT_FALSE(module_->getVersion().empty());
}}

TEST_F({module_name.title()}Test, Initialization) {{
    EXPECT_TRUE(module_->initialize());
    
    // 重复初始化应该成功
    EXPECT_TRUE(module_->initialize());
}}

TEST_F({module_name.title()}Test, BasicFunctionality) {{
    ASSERT_TRUE(module_->initialize());
    
    // TODO: 添加具体的功能测试
    EXPECT_EQ(module_->getName(), "{module_name}");
}}

}} // namespace fq::test
"""
        
        test_file = test_dir / f"test_{module_name}.cpp"
        test_file.write_text(content, encoding='utf-8')
    
    def _update_main_cmake(self, module_name: str):
        """更新主CMakeLists.txt文件"""
        cmake_file = self.src_dir / "CMakeLists.txt"
        
        if not cmake_file.exists():
            return
        
        content = cmake_file.read_text(encoding='utf-8')
        new_line = f"add_subdirectory({module_name})"
        
        if new_line not in content:
            content += f"\n{new_line}\n"
            cmake_file.write_text(content, encoding='utf-8')
            print(f"✅ 已更新 {cmake_file}")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="FastQTools 模块生成器")
    parser.add_argument("module_name", help="模块名称 (小写)")
    parser.add_argument("--type", choices=["library", "executable"], 
                       default="library", help="模块类型")
    parser.add_argument("--project-root", type=Path, 
                       default=Path(__file__).parent.parent.parent,
                       help="项目根目录")
    
    args = parser.parse_args()
    
    # 验证模块名称
    if not args.module_name.islower():
        print("❌ 模块名称必须是小写")
        return 1
    
    if not args.module_name.isidentifier():
        print("❌ 模块名称必须是有效的标识符")
        return 1
    
    # 创建生成器并生成模块
    generator = ModuleGenerator(args.project_root)
    
    if generator.create_module(args.module_name, args.type):
        print(f"\\n🎉 模块 '{args.module_name}' 创建完成!")
        print("\\n📝 下一步:")
        print(f"1. 编辑 src/{args.module_name}/{args.module_name}.h 定义接口")
        print(f"2. 实现 src/{args.module_name}/{args.module_name}.cpp 中的功能")
        print(f"3. 完善 tests/unit/{args.module_name}/test_{args.module_name}.cpp 中的测试")
        print("4. 运行 ./scripts/build.sh --test 验证构建")
        return 0
    else:
        return 1

if __name__ == "__main__":
    sys.exit(main())
"""

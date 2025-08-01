
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.cmake import cmake_layout
import os

class FastQTools(ConanFile):
    name = "fastqtools"
    version = "3.1.0"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "app/*", "cmake/*", "vcpkg.json"

    def requirements(self):
        """
        Declare all dependencies for the project.
        """
        self.requires("cxxopts/3.1.1")
        self.requires("spdlog/1.12.0")
        self.requires("zlib/1.3")
        self.requires("bzip2/1.0.8")
        self.requires("xz_utils/5.4.5")
        self.requires("nlohmann_json/3.11.3")
        self.requires("fmt/10.2.1")
        # High-performance memory allocator
        self.requires("mimalloc/2.1.2")
        # Intel's Threading Building Blocks for high-level parallelism
        self.requires("onetbb/2021.10.0")

    def build_requirements(self):
        """
        Dependencies required only for building the project, like testing frameworks.
        """
        self.tool_requires("cmake/[>=3.20]")
        if self.options.get_safe("build_testing", True):
            self.requires("gtest/1.14.0")

    

    def generate(self):
        """
        Generate build system files.
        """
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        
        # Generate CMake toolchain file
        tc.generate()
        
        # Additional CMake configuration for Clang
        if self.settings.compiler == "clang":
            toolchain_path = os.path.join(self.generators_folder, "conan_toolchain.cmake")
            with open(toolchain_path, "a") as f:
                f.write("""
# Fix Clang-specific compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
""")

    def build(self):
        """
        Build the project using CMake.
        """
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        """
        Package the project artifacts.
        """
        cmake = CMake(self)
        cmake.install()

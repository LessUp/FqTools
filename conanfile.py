# conanfile.py
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class FastQToolsConan(ConanFile):
    name = "fastqtools"
    version = "3.1.0"
    license = "MIT" # Please update if not correct
    author = "Your Name <you@example.com>" # Please update
    url = "https://github.com/your-org/fastqtools" # Please update
    description = "A modern toolkit for FASTQ file processing"
    topics = ("bioinformatics", "fastq", "genomics")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "CMakeLists.txt", "src/*", "cmake/*", "third_party/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["fq_lib", "fq_cli"] # Add all relevant library targets
        self.cpp_info.set_property("cmake_file_name", "FastQTools")
        self.cpp_info.set_property("cmake_target_name", "FastQTools::FastQTools")

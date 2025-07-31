# dist/homebrew/fastqtools.rb
class Fastqtools < Formula
  desc "A modern toolkit for FASTQ file processing"
  homepage "https://github.com/your-org/fastqtools" # Please update
  url "https://github.com/your-org/fastqtools/archive/refs/tags/v2.0.0.tar.gz" # Placeholder
  sha256 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" # Placeholder for empty file
  license "MIT" # Please update if not correct

  depends_on "cmake" => :build
  depends_on "conan" => :build
  depends_on "python@3.11" => :build

  def install
    # Set up Conan
    system "conan", "profile", "detect", "--force"

    # Install dependencies using Conan
    system "conan", "install", "config/dependencies/", "--output-folder=build", "--build=missing", "-s", "build_type=Release"

    # Configure and build with CMake
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args, "-DCMAKE_BUILD_TYPE=Release"
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/FastQTools", "--version"
  end
end

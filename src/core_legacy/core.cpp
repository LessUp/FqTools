/**
 * @file Core.cpp
 * @brief 核心模块实现文件
 * @details 实现了FastQTools项目的核心功能，包括通用工具函数、计时器、编码器、FASTQ文件处理等
 * 
 * @author FastQTools Team
 * @date 2025-07-31
 * @version 3.1.0
 * 
 * @copyright Copyright (c) 2025 BGI-Research
 */

#include "core_legacy/core.h"

namespace fq::common {


/**
 * @brief 获取当前时间字符串
 * @details 获取格式化的当前时间字符串，使用C++标准库的时间函数
 * 
 * @param fmt 时间格式字符串，默认为"%Y-%m-%d %H:%M:%S"
 * @return 格式化的时间字符串，失败时返回空字符串
 */
auto getCurrentTime(const std::string& fmt) -> std::string {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
        constexpr int TIME_BUFFER_SIZE = 64;
    char buf[TIME_BUFFER_SIZE];
    if (std::strftime(buf, sizeof(buf), fmt.c_str(), std::localtime(&in_time_t))) {
        return buf;
    }
    return "";
}

/**
 * @brief 去除字符串首尾空格
 * @details 去除输入字符串首尾的空白字符（空格、制表符、换行符、回车符）
 * 
 * @param str 输入字符串
 * @return 去除首尾空格后的字符串，如果字符串全为空格则返回空字符串
 */
auto trimSpace(std::string_view str) -> std::string {
    auto first = str.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) return "";
    auto last = str.find_last_not_of(" \t\n\r");
    return std::string(str.substr(first, (last - first + 1)));
}


/**
 * @brief 构造函数
 * @details 创建计时器实例，记录开始时间并设置计时器名称
 * 
 * @param name 计时器名称，用于标识不同的计时器
 */
Timer::Timer(std::string name) : m_name(std::move(name)), m_start_time(std::chrono::high_resolution_clock::now()) {}

/**
 * @brief 报告计时结果
 * @details 计算从开始到当前的时间差，并以日志形式输出
 * 
 * @param is_debug 是否以调试级别输出，默认为true；false时以信息级别输出
 */
void Timer::report(bool is_debug) {
    const auto end_time = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> diff = end_time - m_start_time;
    if (is_debug) {
        spdlog::debug("Timer[{}]: interval time:{:.3f}s", m_name, diff.count());
    } else {
        spdlog::info("Timer[{}]: interval time:{:.3f}s", m_name, diff.count());
    }
}

/**
 * @brief 输出软件信息
 * @details 向标准错误输出流输出软件的基本信息，包括程序名称、版本、分支和提交哈希
 * 
 * @param soft_name 软件名称
 */
void software_info(const char* soft_name) {
    fprintf(stderr, "\n");
    fprintf(stderr, "== Program   : %s\n", soft_name);
    fprintf(stderr, "== Version   : %s\n", "dev");
    fprintf(stderr, "== Branch    : %s\n", "unknown");
    fprintf(stderr, "== CommitHash: %s\n\n", "unknown");
}

/**
 * @brief 打印大型Logo
 * @details 在控制台输出软件的ASCII艺术Logo（占位符实现）
 * 
 * @param color 是否使用彩色输出，默认为true
 */
void print_big_logo(bool /*color*/) { /* Implementation placeholder */ }

/**
 * @brief 打印软件信息
 * @details 向标准输出流输出软件的版本信息和描述
 */
void printSoftwareInfo() {
    std::cout << "=================================================================================" << std::endl;
    std::cout << "FastQTools - A toolkit for FASTQ file processing" << std::endl;
    std::cout << "Version: " << "dev" << std::endl;
    std::cout << "=================================================================================" << std::endl;
}

/**
 * @brief 彩色打印文本
 * @details 在支持的终端上以指定颜色打印文本，支持Windows和Unix系统
 * 
 * @param text 要打印的文本
 * @param color 颜色代码（Windows为控制台颜色值，Unix为ANSI颜色代码）
 */
void printColor(const std::string& text, int color) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    std::cout << text;
    SetConsoleTextAttribute(hConsole, 15); // Reset to white
#else
    std::cout << "\033[" << color << "m" << text << "\033[0m";
#endif
}

/**
 * @brief 获取日志记录器
 * @details 获取全局共享的日志记录器实例，使用静态局部变量确保单例模式
 * 
 * @return 日志记录器的共享指针
 */
auto getLogger() -> std::shared_ptr<spdlog::logger> {
    static auto logger = spdlog::stdout_color_mt("fastqtools");
    return logger;
}

/**
 * @brief 初始化日志记录器
 * @details 初始化指定名称的日志记录器并设置为调试级别
 * 
 * @param name 日志记录器名称，默认为"fastqtools"
 */
void initLogger(const std::string& name) {
    auto logger = spdlog::stdout_color_mt(name);
    logger->set_level(spdlog::level::debug);
}

} // namespace fq::common

namespace fq::encoder {

/**
 * @brief 获取压缩参数
 * @details 根据压缩级别返回相应的压缩参数值
 * 
 * @return 压缩参数值（Fast=1, Default=6, High=9）
 */
auto EncoderContext::getCompressionParam() const -> int {
    switch (level) {
        case CompressionLevel::Fast:    return 1;
        case CompressionLevel::Default: return 6;
        case CompressionLevel::High:    return 9;
        default:                        return 6;
    }
}

/**
 * @brief 构造函数
 * @details 初始化ID压缩器，预分配字典和最后部分的存储空间
 */
IDCompressor::IDCompressor() { 
    constexpr size_t INITIAL_DICTIONARY_CAPACITY = 1024;
    constexpr size_t INITIAL_LAST_PARTS_CAPACITY = 16;
    m_dictionary.reserve(INITIAL_DICTIONARY_CAPACITY); 
    m_last_parts.reserve(INITIAL_LAST_PARTS_CAPACITY); 
}

/**
 * @brief 压缩ID数据
 * @details 基础实现，当前仅复制数据（占位符实现）
 * 
 * @param raw_data 原始数据
 * @param compressed_data 压缩后的数据
 * @param context 编码器上下文
 */
void IDCompressor::compress(const std::vector<char>& raw_data, std::vector<char>& compressed_data, const EncoderContext& /*context*/) {
    // Basic implementation - just copy data for now
    compressed_data = raw_data;
}

/**
 * @brief 解压缩ID数据
 * @details 基础实现，当前仅复制数据（占位符实现）
 * 
 * @param compressed_data 压缩数据
 * @param raw_data 解压缩后的数据
 * @param context 编码器上下文
 */
void IDCompressor::decompress(const std::vector<char>& compressed_data, std::vector<char>& raw_data, const EncoderContext& /*context*/) {
    // Basic implementation - just copy data for now
    raw_data = compressed_data;
}

/**
 * @brief 构造函数
 * @details 初始化质量分数压缩器
 */
QualCompressor::QualCompressor() {}

void QualCompressor::compress(const std::vector<char>& raw_data, std::vector<char>& compressed_data, const EncoderContext& /*context*/) {
    // Basic implementation - just copy data for now
    compressed_data = raw_data;
}

void QualCompressor::decompress(const std::vector<char>& compressed_data, std::vector<char>& raw_data, const EncoderContext& /*context*/) {
    // Basic implementation - just copy data for now
    raw_data = compressed_data;
}

EncoderPipeline::EncoderPipeline(const EncoderContext& context)
    : m_context(context),
      m_id_compressor(std::make_unique<IDCompressor>()),
      m_qual_compressor(std::make_unique<QualCompressor>()) {}

void EncoderPipeline::run(const std::vector<char>& id_data, const std::vector<char>& qual_data) {
    m_id_compressor->compress(id_data, m_compressed_id_data, m_context);
    m_qual_compressor->compress(qual_data, m_compressed_qual_data, m_context);
}

auto EncoderPipeline::getCompressedIDData() const -> const std::vector<char>& { return m_compressed_id_data; }
auto EncoderPipeline::getCompressedQualData() const -> const std::vector<char>& { return m_compressed_qual_data; }

} // namespace fq::encoder

namespace fq::fastq {

FastQInfer::FastQInfer(const std::string& /*input_path*/, uint32_t /*infer_batch_size*/) { /* Placeholder */ }

auto FastQInfer::getFqFileAttribution() const -> const FqFileAttribution& { return m_fqfile_attribution; }

void FastQInfer::setFqScoreType(QScoreType q_score_type) { m_fqfile_attribution.q_score_type = q_score_type; }

void FastQInfer::setQ4Rule(bool q4_rule) { m_fqfile_attribution.force_q4_rule = q4_rule; }

FastQReader::FastQReader(std::string file_name, std::shared_ptr<FastQInfer> fq_infer, bool enable_validation)
    : m_file_name1(std::move(file_name)), m_fq_infer(std::move(fq_infer)), m_validation_enabled(enable_validation) {
    openFile(m_file_name1, m_stream1);
}

FastQReader::FastQReader(std::string file_name1, std::string file_name2, std::shared_ptr<FastQInfer> fq_infer, bool enable_validation)
    : m_file_name1(std::move(file_name1)), m_file_name2(std::move(file_name2)), m_fq_infer(std::move(fq_infer)), m_validation_enabled(enable_validation) {
    openFile(m_file_name1, m_stream1);
    openFile(m_file_name2, m_stream2);
    m_is_pe_mode = true;
}

FastQReader::~FastQReader() = default;

auto FastQReader::read(FqInfoBatch& /*batch*/, int /*batch_size*/) -> bool {
    // Basic implementation - return false for now
    return false;
}

auto FastQReader::isOpened() -> bool { return m_stream1 != nullptr; }

auto FastQReader::eof() const -> bool { 
    return m_stream1 ? m_stream1->eof() : true; 
}

auto FastQReader::getReadLen() const -> uint32_t { return 0; }

auto FastQReader::getQualitySystem() const -> QScoreType { return QScoreType::Unknown; }

void FastQReader::openFile(const std::string& file_name, std::unique_ptr<igzstream>& stream_ptr) {
    stream_ptr = std::make_unique<igzstream>(file_name.c_str());
}

auto FastQReader::getNextRecord(FqInfo& /*record*/, igzstream& /*stream*/) -> FQReadState {
    return FQReadState::End;
}

FastQWriter::FastQWriter(const std::string& file_name) : m_file_name(file_name) {
    m_stream = std::make_unique<ogzstream>(file_name.c_str());
}

FastQWriter::~FastQWriter() = default;

void FastQWriter::write(const FqInfoBatch& /*batch*/) {
    // Basic implementation - do nothing for now
}

auto FastQWriter::isOpened() -> bool { return m_stream != nullptr; }

} // namespace fq::fastq

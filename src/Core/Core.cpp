#include "Core/Core.h"

namespace fq::common {

void split(std::string_view str, std::vector<std::string>& tokens, std::string_view delim) {
    tokens.clear();
    size_t lastPos = str.find_first_not_of(delim, 0);
    size_t pos = str.find(delim, lastPos);
    while (lastPos != std::string_view::npos) {
        tokens.emplace_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delim, pos);
        pos = str.find(delim, lastPos);
    }
}

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

auto trimSpace(std::string_view str) -> std::string {
    auto first = str.find_first_not_of(" \t\n\r");
    if (first == std::string_view::npos) return "";
    auto last = str.find_last_not_of(" \t\n\r");
    return std::string(str.substr(first, (last - first + 1)));
}

auto split(std::string_view str, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    auto view = str | std::views::split(delimiter) | std::views::transform([](auto&& range) {
        return std::string(range.begin(), range.end());
    });
    std::ranges::copy(view, std::back_inserter(tokens));
    return tokens;
}

Timer::Timer(std::string name) : m_name(std::move(name)), m_start_time(std::chrono::high_resolution_clock::now()) {}

void Timer::report(bool is_debug) {
    const auto end_time = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> diff = end_time - m_start_time;
    if (is_debug) {
        spdlog::debug("Timer[{}]: interval time:{:.3f}s", m_name, diff.count());
    } else {
        spdlog::info("Timer[{}]: interval time:{:.3f}s", m_name, diff.count());
    }
}

void software_info(const char* soft_name) {
    fprintf(stderr, "\n");
    fprintf(stderr, "== Program   : %s\n", soft_name);
    fprintf(stderr, "== Version   : %s\n", "dev");
    fprintf(stderr, "== Branch    : %s\n", "unknown");
    fprintf(stderr, "== CommitHash: %s\n\n", "unknown");
}

void print_big_logo(bool color) { /* Implementation placeholder */ }

void printSoftwareInfo() {
    std::cout << "=================================================================================" << std::endl;
    std::cout << "FastQTools - A toolkit for FASTQ file processing" << std::endl;
    std::cout << "Version: " << "dev" << std::endl;
    std::cout << "=================================================================================" << std::endl;
}

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

auto getLogger() -> std::shared_ptr<spdlog::logger> {
    static auto logger = spdlog::stdout_color_mt("fastqtools");
    return logger;
}

void initLogger(const std::string& name) {
    auto logger = spdlog::stdout_color_mt(name);
    logger->set_level(spdlog::level::debug);
}

} // namespace fq::common

namespace fq::encoder {

auto EncoderContext::getCompressionParam() const -> int {
    switch (level) {
        case CompressionLevel::Fast:    return 1;
        case CompressionLevel::Default: return 6;
        case CompressionLevel::High:    return 9;
        default:                        return 6;
    }
}

IDCompressor::IDCompressor() { 
    constexpr size_t INITIAL_DICTIONARY_CAPACITY = 1024;
    constexpr size_t INITIAL_LAST_PARTS_CAPACITY = 16;
    m_dictionary.reserve(INITIAL_DICTIONARY_CAPACITY); 
    m_last_parts.reserve(INITIAL_LAST_PARTS_CAPACITY); 
}

void IDCompressor::compress(const std::vector<char>& raw_data, std::vector<char>& compressed_data, const EncoderContext& context) {
    // Basic implementation - just copy data for now
    compressed_data = raw_data;
}

void IDCompressor::decompress(const std::vector<char>& compressed_data, std::vector<char>& raw_data, const EncoderContext& context) {
    // Basic implementation - just copy data for now
    raw_data = compressed_data;
}

QualCompressor::QualCompressor() {}

void QualCompressor::compress(const std::vector<char>& raw_data, std::vector<char>& compressed_data, const EncoderContext& context) {
    // Basic implementation - just copy data for now
    compressed_data = raw_data;
}

void QualCompressor::decompress(const std::vector<char>& compressed_data, std::vector<char>& raw_data, const EncoderContext& context) {
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

FastQInfer::FastQInfer(const std::string& input_path, uint32_t infer_batch_size) { /* Placeholder */ }

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

auto FastQReader::read(FqInfoBatch& batch, int batch_size) -> bool {
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

auto FastQReader::getNextRecord(FqInfo& record, igzstream& stream) -> FQReadState {
    return FQReadState::End;
}

FastQWriter::FastQWriter(const std::string& file_name) : m_file_name(file_name) {
    m_stream = std::make_unique<ogzstream>(file_name.c_str());
}

FastQWriter::~FastQWriter() = default;

void FastQWriter::write(const FqInfoBatch& batch) {
    // Basic implementation - do nothing for now
}

auto FastQWriter::isOpened() -> bool { return m_stream != nullptr; }

} // namespace fq::fastq

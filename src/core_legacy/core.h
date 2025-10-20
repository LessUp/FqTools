/**
 * @file Core.h
 * @brief 核心模块头文件
 * @details 该文件包含了FastQTools项目的核心功能类和结构体定义，包括配置、公共工具、FASTQ处理和编码器等模块
 *
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 *
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once

#include "gzstream.h"
#include "std.h"

namespace fq
{

//==============================================================================
// From Common.cppm
//==============================================================================
namespace config
{
constexpr int ERR_CODE = 255;
constexpr int FASTQ_LINES_PER_RECORD = 4;
} // namespace config

namespace common
{

/**
 * @brief 获取当前时间字符串
 * @details 获取格式化的当前时间字符串
 *
 * @param fmt 时间格式字符串，默认为"%Y-%m-%d %H:%M:%S"
 * @return 格式化的时间字符串
 */
auto getCurrentTime(const std::string &fmt = "%Y-%m-%d %H:%M:%S") -> std::string;

/**
 * @brief 去除字符串首尾空格
 * @details 去除输入字符串首尾的空白字符
 *
 * @param str 输入字符串
 * @return 去除首尾空格后的字符串
 */
auto trimSpace(std::string_view str) -> std::string;


/**
 * @brief 计时器类
 * @details 用于测量代码执行时间的工具类
 */
class Timer
{
public:
    /**
     * @brief 构造函数
     * @details 创建计时器并开始计时
     *
     * @param name 计时器名称
     */
    explicit Timer(std::string name);

    /**
     * @brief 报告计时结果
     * @details 输出计时结果到日志
     *
     * @param is_debug 是否以调试级别输出，默认为true
     */
    void report(bool is_debug = true);

private:
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

constexpr int32_t ID_EXIT_SIGNAL = -1;

/**
 * @brief 带ID的基类
 * @details 为对象提供唯一ID标识的基类
 */
class WithID
{
public:
    virtual ~WithID() = default;
    uint32_t id = 0; ///< 对象ID
};

/**
 * @brief 输出软件信息
 * @details 打印软件版本和版权信息
 *
 * @param soft_name 软件名称
 */
void software_info(const char *soft_name);

/**
 * @brief 打印大型Logo
 * @details 在控制台输出软件的ASCII艺术Logo
 *
 * @param color 是否使用彩色输出，默认为true
 */
void print_big_logo(bool color = true);

/**
 * @brief 打印软件信息
 * @details 打印软件版本信息
 */
void printSoftwareInfo();

/**
 * @brief 彩色打印文本
 * @details 在支持的终端上以指定颜色打印文本
 *
 * @param text 要打印的文本
 * @param color 颜色代码
 */
void printColor(const std::string &text, int color);

/**
 * @brief 获取日志记录器
 * @details 获取全局共享的日志记录器实例
 *
 * @return 日志记录器的共享指针
 */
auto getLogger() -> std::shared_ptr<spdlog::logger>;

/**
 * @brief 初始化日志记录器
 * @details 初始化全局日志记录器
 *
 * @param name 日志记录器名称，默认为"fastqtools"
 */
void initLogger(const std::string &name = "fastqtools");
} // namespace common

//==============================================================================
// From FastQ.cppm
//==============================================================================
namespace fastq
{
/**
 * @brief 质量分数类型枚举
 * @details 定义了不同测序平台的质量分数编码类型
 */
enum class QScoreType
{
    Unknown,    ///< 未知类型
    Sanger,     ///< Sanger类型，偏移量为33
    Illumina13, ///< Illumina 1.3+类型，偏移量为64
    Illumina15, ///< Illumina 1.5+类型，偏移量为64
    Illumina18, ///< Illumina 1.8+类型，偏移量为33
    MGI,        ///< MGI类型
    NovaSeqQ4,  ///< NovaSeq Q4类型
    MGIQ4       ///< MGI Q4类型
};

/**
 * @brief 测序数据生成类型枚举
 * @details 定义了测序数据的生成技术类型
 */
enum class SequencingDataGeneration
{
    Second, ///< 第二代测序
    Third   ///< 第三代测序
};

/**
 * @brief FASTQ读取状态枚举
 * @details 定义了FASTQ文件读取操作的返回状态
 */
enum class FQReadState
{
    Error = -1, ///< 读取错误
    End = 0,    ///< 文件结束
    Success = 1 ///< 读取成功
};

/**
 * @brief FASTQ信息结构体
 * @details 存储单个FASTQ读取记录的信息，包括名称、序列和质量分数
 */
struct FqInfo
{
    std::string name; ///< 读取名称
    std::string base; ///< 碱基序列
    std::string qual; ///< 质量分数字符串
};

/**
 * @brief FASTQ信息批次类
 * @details 继承自WithID类，用于存储一批FASTQ读取记录
 */
class FqInfoBatch : public common::WithID
{
public:
    std::vector<FqInfo> reads; ///< 读取记录向量

    /**
     * @brief 清空批次数据
     * @details 清空所有读取记录并将ID重置为0
     */
    void clear()
    {
        reads.clear();
        id = 0;
    }

    /**
     * @brief 获取批次大小
     * @details 返回当前批次中读取记录的数量
     *
     * @return 读取记录数量
     */
    [[nodiscard]] auto size() const -> size_t { return reads.size(); }

    /**
     * @brief 检查批次是否为空
     * @details 判断当前批次是否没有读取记录
     *
     * @return 为空返回true，否则返回false
     */
    [[nodiscard]] auto empty() const -> bool { return reads.empty(); }
};

/**
 * @brief FASTQ文件属性结构体
 * @details 存储FASTQ文件的各种属性信息
 */
struct FqFileAttribution
{
    uint32_t read_length = 0;                                                    ///< 读取长度
    QScoreType q_score_type = QScoreType::Unknown;                               ///< 质量分数类型
    bool is_mutable_read_length = false;                                         ///< 是否可变读取长度
    uint32_t qname_length = 0;                                                   ///< 读取名称长度
    bool is_line3_dup = false;                                                   ///< 第三行是否重复
    bool is_RNA = false;                                                         ///< 是否为RNA数据
    bool is_q4 = false;                                                          ///< 是否为Q4质量分数
    SequencingDataGeneration data_generation = SequencingDataGeneration::Second; ///< 数据生成类型
    uint32_t max_read_length = 0;                                                ///< 最大读取长度
    uint32_t fq_length = 0;                                                      ///< FASTQ记录长度
    bool force_q4_rule = false;                                                  ///< 是否强制使用Q4规则
};

// Basic FastQ classes with minimal implementation
/**
 * @brief FASTQ推断器类
 * @details 用于推断FASTQ文件的属性信息
 */
class FastQInfer
{
public:
    /**
     * @brief 构造函数
     * @details 创建FASTQ推断器并初始化
     *
     * @param input_path 输入文件路径
     * @param infer_batch_size 推断批处理大小，默认为10000
     */
    explicit FastQInfer(const std::string &input_path, uint32_t infer_batch_size = 10000);

    /**
     * @brief 获取FASTQ文件属性
     * @details 返回推断得到的FASTQ文件属性信息
     *
     * @return 文件属性引用
     */
    [[nodiscard]] auto getFqFileAttribution() const -> const FqFileAttribution &;

    /**
     * @brief 设置质量分数类型
     * @details 手动设置FASTQ文件的质量分数类型
     *
     * @param q_score_type 质量分数类型
     */
    void setFqScoreType(QScoreType q_score_type);

    /**
     * @brief 设置Q4规则
     * @details 设置是否使用Q4质量分数规则
     *
     * @param q4_rule 是否使用Q4规则
     */
    void setQ4Rule(bool q4_rule);

private:
    FqFileAttribution m_fqfile_attribution; ///< 文件属性
};

/**
 * @brief FASTQ读取器类
 * @details 用于读取FASTQ文件中的数据
 */
class FastQReader
{
public:
    /**
     * @brief 单端文件构造函数
     * @details 创建单端FASTQ文件读取器
     *
     * @param file_name 文件名
     * @param fq_infer FASTQ推断器指针，默认为nullptr
     * @param enable_validation 是否启用验证，默认为false
     */
    explicit FastQReader(std::string file_name, std::shared_ptr<FastQInfer> fq_infer = nullptr,
                         bool enable_validation = false);

    /**
     * @brief 双端文件构造函数
     * @details 创建双端FASTQ文件读取器
     *
     * @param file_name1 第一个文件名
     * @param file_name2 第二个文件名
     * @param fq_infer FASTQ推断器指针，默认为nullptr
     * @param enable_validation 是否启用验证，默认为false
     */
    FastQReader(std::string file_name1, std::string file_name2, std::shared_ptr<FastQInfer> fq_infer = nullptr,
                bool enable_validation = false);

    /**
     * @brief 析构函数
     * @details 释放读取器资源
     */
    ~FastQReader();

    /**
     * @brief 读取批次数据
     * @details 从文件中读取指定数量的读取记录到批次中
     *
     * @param batch 读取批次引用
     * @param batch_size 批次大小
     * @return 读取成功返回true，失败返回false
     */
    auto read(FqInfoBatch &batch, int batch_size) -> bool;

    /**
     * @brief 检查文件是否已打开
     * @details 判断FASTQ文件是否成功打开
     *
     * @return 已打开返回true，否则返回false
     */
    [[nodiscard]] auto isOpened() -> bool;

    /**
     * @brief 检查是否到达文件末尾
     * @details 判断是否已读取到文件末尾
     *
     * @return 到达末尾返回true，否则返回false
     */
    [[nodiscard]] auto eof() const -> bool;

    /**
     * @brief 获取读取长度
     * @details 返回文件中读取记录的长度
     *
     * @return 读取长度
     */
    [[nodiscard]] auto getReadLen() const -> uint32_t;

    /**
     * @brief 获取质量分数系统
     * @details 返回文件中使用的质量分数类型
     *
     * @return 质量分数类型
     */
    [[nodiscard]] auto getQualitySystem() const -> QScoreType;

private:
    /**
     * @brief 打开文件
     * @details 打开指定的FASTQ文件
     *
     * @param file_name 文件名
     * @param stream_ptr 文件流指针引用
     */
    void openFile(const std::string &file_name, std::unique_ptr<igzstream> &stream_ptr);

    /**
     * @brief 获取下一个记录
     * @details 从文件流中读取下一个FASTQ记录
     *
     * @param record 读取记录引用
     * @param stream 文件流引用
     * @return 读取状态
     */
    auto getNextRecord(FqInfo &record, igzstream &stream) -> FQReadState;

    std::string m_file_name1;                      ///< 第一个文件名
    std::string m_file_name2;                      ///< 第二个文件名
    std::unique_ptr<igzstream> m_stream1;          ///< 第一个文件流
    std::unique_ptr<igzstream> m_stream2;          ///< 第二个文件流
    std::shared_ptr<FastQInfer> m_fq_infer;        ///< FASTQ推断器
    bool m_is_pe_mode = false;                     ///< 是否为双端模式
    bool m_validation_enabled = false;             ///< 是否启用验证
    class CheckFqRules *m_rules_checker = nullptr; ///< 规则检查器
};

/**
 * @brief FASTQ写入器类
 * @details 用于将FASTQ数据写入文件
 */
class FastQWriter
{
public:
    /**
     * @brief 构造函数
     * @details 创建FASTQ文件写入器
     *
     * @param file_name 文件名
     */
    explicit FastQWriter(const std::string &file_name);

    /**
     * @brief 析构函数
     * @details 释放写入器资源
     */
    ~FastQWriter();

    /**
     * @brief 写入批次数据
     * @details 将批次中的读取记录写入文件
     *
     * @param batch 读取批次引用
     */
    void write(const FqInfoBatch &batch);

    /**
     * @brief 检查文件是否已打开
     * @details 判断FASTQ文件是否成功打开
     *
     * @return 已打开返回true，否则返回false
     */
    [[nodiscard]] auto isOpened() -> bool;

private:
    std::string m_file_name;             ///< 文件名
    std::unique_ptr<ogzstream> m_stream; ///< 文件流
};

constexpr int MAX_QUAL = 42;                  ///< 最大质量分数值
constexpr double MAX_PHRED_SCORE = 93.0;      ///< 最大Phred分数值
constexpr int PHRED_OFFSET_SANGER = 33;       ///< Sanger偏移量
constexpr int PHRED_OFFSET_ILLUMINA_1_3 = 64; ///< Illumina 1.3+偏移量
} // namespace fastq

//==============================================================================
// From Encoder.cppm
//==============================================================================
namespace encoder
{
/**
 * @brief 压缩级别枚举
 * @details 定义了数据压缩的级别选项
 */
enum class CompressionLevel
{
    Fast,    ///< 快速压缩
    Default, ///< 默认压缩
    High     ///< 高压缩比
};

/**
 * @brief 编码器上下文结构体
 * @details 存储编码器的配置参数和上下文信息
 */
struct EncoderContext
{
    CompressionLevel level = CompressionLevel::Default; ///< 压缩级别
    uint32_t thread_num = 1;                            ///< 线程数量
    bool enable_validation = false;                     ///< 是否启用验证
    size_t buffer_size = 64 * 1024;                     ///< 缓冲区大小

    /**
     * @brief 获取压缩参数
     * @details 根据压缩级别返回相应的压缩参数
     *
     * @return 压缩参数值
     */
    auto getCompressionParam() const -> int;
};

/**
 * @brief 压缩器接口类
 * @details 定义了数据压缩和解压缩的接口，继承自WithID类
 */
class ICompressor : public common::WithID
{
public:
    virtual ~ICompressor() = default;

    /**
     * @brief 压缩数据
     * @details 将原始数据压缩为压缩数据
     *
     * @param raw_data 原始数据
     * @param compressed_data 压缩后的数据
     * @param context 编码器上下文
     */
    virtual void compress(const std::vector<char> &raw_data, std::vector<char> &compressed_data,
                          const EncoderContext &context) = 0;

    /**
     * @brief 解压缩数据
     * @details 将压缩数据解压缩为原始数据
     *
     * @param compressed_data 压缩数据
     * @param raw_data 解压缩后的数据
     * @param context 编码器上下文
     */
    virtual void decompress(const std::vector<char> &compressed_data, std::vector<char> &raw_data,
                            const EncoderContext &context) = 0;
};

/**
 * @brief ID压缩器类
 * @details 实现ID数据的压缩和解压缩功能
 */
class IDCompressor : public ICompressor
{
public:
    /**
     * @brief 构造函数
     * @details 创建ID压缩器并初始化
     */
    IDCompressor();

    /**
     * @brief 压缩数据
     * @details 将原始数据压缩为压缩数据
     *
     * @param raw_data 原始数据
     * @param compressed_data 压缩后的数据
     * @param context 编码器上下文
     */
    void compress(const std::vector<char> &raw_data, std::vector<char> &compressed_data,
                  const EncoderContext &context) override;

    /**
     * @brief 解压缩数据
     * @details 将压缩数据解压缩为原始数据
     *
     * @param compressed_data 压缩数据
     * @param raw_data 解压缩后的数据
     * @param context 编码器上下文
     */
    void decompress(const std::vector<char> &compressed_data, std::vector<char> &raw_data,
                    const EncoderContext &context) override;

private:
    static constexpr uint32_t MAX_DICTIONARY_SIZE = 4096; ///< 最大字典大小
    std::vector<std::string> m_dictionary;                ///< 字典
    std::unordered_map<std::string, uint32_t> m_dict_map; ///< 字典映射
    std::vector<std::string> m_last_parts;                ///< 最后部分
};

/**
 * @brief 质量分数压缩器类
 * @details 实现质量分数数据的压缩和解压缩功能
 */
class QualCompressor : public ICompressor
{
public:
    /**
     * @brief 构造函数
     * @details 创建质量分数压缩器并初始化
     */
    QualCompressor();

    /**
     * @brief 压缩数据
     * @details 将原始数据压缩为压缩数据
     *
     * @param raw_data 原始数据
     * @param compressed_data 压缩后的数据
     * @param context 编码器上下文
     */
    void compress(const std::vector<char> &raw_data, std::vector<char> &compressed_data,
                  const EncoderContext &context) override;

    /**
     * @brief 解压缩数据
     * @details 将压缩数据解压缩为原始数据
     *
     * @param compressed_data 压缩数据
     * @param raw_data 解压缩后的数据
     * @param context 编码器上下文
     */
    void decompress(const std::vector<char> &compressed_data, std::vector<char> &raw_data,
                    const EncoderContext &context) override;
};

/**
 * @brief 编码器流水线类
 * @details 管理ID压缩器和质量分数压缩器的编码流水线
 */
class EncoderPipeline
{
public:
    /**
     * @brief 构造函数
     * @details 创建编码器流水线并初始化
     *
     * @param context 编码器上下文
     */
    EncoderPipeline(const EncoderContext &context);

    /**
     * @brief 运行编码器流水线
     * @details 执行ID数据和质量分数数据的压缩
     *
     * @param id_data ID数据
     * @param qual_data 质量分数数据
     */
    void run(const std::vector<char> &id_data, const std::vector<char> &qual_data);

    /**
     * @brief 获取压缩后的ID数据
     * @details 返回压缩后的ID数据引用
     *
     * @return 压缩后的ID数据引用
     */
    auto getCompressedIDData() const -> const std::vector<char> &;

    /**
     * @brief 获取压缩后的质量分数数据
     * @details 返回压缩后的质量分数数据引用
     *
     * @return 压缩后的质量分数数据引用
     */
    auto getCompressedQualData() const -> const std::vector<char> &;

private:
    EncoderContext m_context;                          ///< 编码器上下文
    std::unique_ptr<IDCompressor> m_id_compressor;     ///< ID压缩器
    std::unique_ptr<QualCompressor> m_qual_compressor; ///< 质量分数压缩器
    std::vector<char> m_compressed_id_data;            ///< 压缩后的ID数据
    std::vector<char> m_compressed_qual_data;          ///< 压缩后的质量分数数据
};
} // namespace encoder

//==============================================================================
// Exception class
//==============================================================================

/**
 * @brief FastQ工具异常类
 * @details 继承自std::runtime_error，用于处理FastQ工具中的运行时异常
 */
class exception : public std::runtime_error
{
public:
    /**
     * @brief 构造函数
     * @details 创建异常对象并初始化错误信息
     *
     * @param message 错误信息
     */
    explicit exception(const std::string &message) : std::runtime_error(message) {}
};

} // namespace fq

#pragma once

// 在完全模块化可行之前，暂时使用传统头文件
// export module fq.core;

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <ranges>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "../common/common.h"
#include "../error/error.h"

// import fq.common;
// import fq.error;

namespace fq::core {
    // 基础ID类型
    using ReadID = std::uint64_t;
    using BatchID = std::uint32_t;
    
    // 质量评分类型
    enum class QScoreType { 
        Unknown = 0, 
        Sanger = 1, 
        Illumina13 = 2, 
        Illumina15 = 3, 
        Illumina18 = 4,
        MGI = 5, 
        NovaSeqQ4 = 6, 
        MGIQ4 = 7 
    };
    
    // 测序代数
    enum class SequencingGeneration { 
        Second = 2, 
        Third = 3 
    };
    
    // 基础接口
    class WithID {
    public:
        virtual ~WithID() = default;
        
        [[nodiscard]] auto id() const noexcept -> ReadID { 
            return m_id; 
        }
        
    protected:
        void set_id(ReadID id) noexcept { 
            m_id = id; 
        }
        
        WithID() : m_id(fq::common::IDGenerator::next_id()) {}
        explicit WithID(ReadID id) : m_id(id) {}
        
    private:
        ReadID m_id;
    };

    // 可克隆接口
    template<typename T>
    class Cloneable {
    public:
        virtual ~Cloneable() = default;
        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<T> = 0;
    };

    // 可序列化接口
    class Serializable {
    public:
        virtual ~Serializable() = default;
        virtual void serialize(std::ostream& os) const = 0;
        virtual void deserialize(std::istream& is) = 0;
    };
    
    // 可验证接口
    class Validatable {
    public:
        virtual ~Validatable() = default;
        [[nodiscard]] virtual auto is_valid() const noexcept -> bool = 0;
        [[nodiscard]] virtual auto validation_errors() const -> std::vector<std::string> { return {}; }
    };
    
    // 内存使用统计接口
    class MemoryTrackable {
    public:
        virtual ~MemoryTrackable() = default;
        [[nodiscard]] virtual auto memory_usage() const noexcept -> std::size_t = 0;
    };
    
    // 统计信息接口
    class Statisticable {
    public:
        virtual ~Statisticable() = default;
        [[nodiscard]] virtual auto get_statistics() const -> std::unordered_map<std::string, std::uint64_t> = 0;
        virtual void reset_statistics() = 0;
    };
    
    // 可配置接口
    class Configurable {
    public:
        virtual ~Configurable() = default;
        virtual void configure(const std::unordered_map<std::string, std::string>& config) = 0;
        [[nodiscard]] virtual auto get_config_schema() const -> std::vector<std::string> { return {}; }
    };
    
    // 质量分数工具
    class QualityScore {
    public:
        static constexpr int MIN_QUALITY = 0;
        static constexpr int MAX_QUALITY = 93;
        static constexpr char MIN_ASCII = '!';
        static constexpr char MAX_ASCII = '~';
        
        // Sanger质量分数转换
        static auto sanger_to_quality(char ascii_char) -> int {
            return static_cast<int>(ascii_char - '!');
        }
        
        static auto quality_to_sanger(int quality) -> char {
            if (quality < MIN_QUALITY || quality > MAX_QUALITY) {
                FQ_THROW_VALIDATION_ERROR("quality", std::to_string(quality));
            }
            return static_cast<char>('!' + quality);
        }
        
        // 计算平均质量分数
        template<std::ranges::range R>
        static auto calculate_average_quality(R&& quality_string) -> double {
            if (std::ranges::empty(quality_string)) {
                return 0.0;
            }
            
            double sum = 0.0;
            std::size_t count = 0;
            
            for (char c : quality_string) {
                sum += sanger_to_quality(c);
                ++count;
            }
            
            return sum / static_cast<double>(count);
        }
        
        // 验证质量字符串
        template<std::ranges::range R>
        static auto is_valid_quality_string(R&& quality_string) -> bool {
            return std::ranges::all_of(quality_string, [](char c) {
                return c >= MIN_ASCII && c <= MAX_ASCII;
            });
        }
    };
    
    // 序列工具
    class SequenceUtils {
    public:
        // 核酸字符集
        static constexpr std::string_view VALID_DNA_CHARS = "ACGTNacgtn";
        static constexpr std::string_view VALID_RNA_CHARS = "ACGUNacgun";
        
        // 验证DNA序列
        template<std::ranges::range R>
        static auto is_valid_dna(R&& sequence) -> bool {
            return std::ranges::all_of(sequence, [](char c) {
                return VALID_DNA_CHARS.find(c) != std::string_view::npos;
            });
        }
        
        // 验证RNA序列
        template<std::ranges::range R>
        static auto is_valid_rna(R&& sequence) -> bool {
            return std::ranges::all_of(sequence, [](char c) {
                return VALID_RNA_CHARS.find(c) != std::string_view::npos;
            });
        }
        
        // 计算GC含量
        template<std::ranges::range R>
        static auto calculate_gc_content(R&& sequence) -> double {
            if (std::ranges::empty(sequence)) {
                return 0.0;
            }
            
            std::size_t gc_count = 0;
            std::size_t total_count = 0;
            
            for (char c : sequence) {
                if (c == 'G' || c == 'C' || c == 'g' || c == 'c') {
                    ++gc_count;
                }
                if (c != 'N' && c != 'n') {
                    ++total_count;
                }
            }
            
            return total_count > 0 ? static_cast<double>(gc_count) / static_cast<double>(total_count) * 100.0 : 0.0;
        }
        
        // 反向互补
        static auto reverse_complement(std::string_view sequence) -> std::string {
            std::string result;
            result.reserve(sequence.size());
            
            for (auto it = sequence.rbegin(); it != sequence.rend(); ++it) {
                char c = *it;
                switch (c) {
                    case 'A': case 'a': result += (c == 'A') ? 'T' : 't'; break;
                    case 'T': case 't': result += (c == 'T') ? 'A' : 'a'; break;
                    case 'G': case 'g': result += (c == 'G') ? 'C' : 'c'; break;
                    case 'C': case 'c': result += (c == 'C') ? 'G' : 'g'; break;
                    case 'N': case 'n': result += c; break;
                    default: result += c; break;
                }
            }
            
            return result;
        }
        
        // 序列复杂度计算（基于Shannon熵）
        template<std::ranges::range R>
        static auto calculate_complexity(R&& sequence) -> double {
            if (std::ranges::empty(sequence)) {
                return 0.0;
            }
            
            std::unordered_map<char, std::size_t> counts;
            std::size_t total = 0;
            
            for (char c : sequence) {
                ++counts[std::toupper(c)];
                ++total;
            }
            
            double entropy = 0.0;
            for (const auto& [base, count] : counts) {
                if (count > 0) {
                    double p = static_cast<double>(count) / static_cast<double>(total);
                    entropy -= p * std::log2(p);
                }
            }
            
            return entropy;
        }
    };
    
    // 性能统计工具
    class PerformanceMetrics {
    public:
        struct Metrics {
            std::chrono::nanoseconds processing_time{0};
            std::size_t items_processed = 0;
            std::size_t bytes_processed = 0;
            std::size_t peak_memory_usage = 0;
            
            [[nodiscard]] auto items_per_second() const -> double {
                if (processing_time.count() == 0) return 0.0;
                return static_cast<double>(items_processed) / 
                       (static_cast<double>(processing_time.count()) / 1e9);
            }
            
            [[nodiscard]] auto bytes_per_second() const -> double {
                if (processing_time.count() == 0) return 0.0;
                return static_cast<double>(bytes_processed) / 
                       (static_cast<double>(processing_time.count()) / 1e9);
            }
            
            [[nodiscard]] auto megabytes_per_second() const -> double {
                return bytes_per_second() / (1024.0 * 1024.0);
            }
        };
        
        void start_timing() {
            m_start_time = std::chrono::high_resolution_clock::now();
        }
        
        void stop_timing() {
            auto end_time = std::chrono::high_resolution_clock::now();
            m_metrics.processing_time += end_time - m_start_time;
        }
        
        void add_items_processed(std::size_t count) {
            m_metrics.items_processed += count;
        }
        
        void add_bytes_processed(std::size_t bytes) {
            m_metrics.bytes_processed += bytes;
        }
        
        void update_peak_memory(std::size_t current_memory) {
            m_metrics.peak_memory_usage = std::max(m_metrics.peak_memory_usage, current_memory);
        }
        
        [[nodiscard]] auto get_metrics() const -> const Metrics& {
            return m_metrics;
        }
        
        void reset() {
            m_metrics = Metrics{};
        }
        
    private:
        Metrics m_metrics;
        std::chrono::high_resolution_clock::time_point m_start_time;
    };
}
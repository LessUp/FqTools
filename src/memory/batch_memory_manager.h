#pragma once

/**
 * @file batch_memory_manager.h
 * @brief 批处理内存管理器
 * @details 该模块实现了高效的内存管理机制，包括对象池、内存监控和自动优化功能
 *          主要用于 FastQ 数据处理过程中的内存分配和释放管理
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "core_legacy/core.h"

namespace fq::memory {

/**
 * @brief FqInfoBatch 对象池，用于重用批处理对象以减少内存分配开销
 * @details 该类实现了对象池模式，用于管理 FqInfoBatch 对象的生命周期
 *          通过重用已分配的对象来减少内存分配和释放的开销
 * 
 * @invariant 池大小不超过最大限制
 * @warning 对象池不保证对象的状态，使用前需要重置
 */
class FqInfoBatchPool {
public:
    /**
     * @brief 构造函数
     * @details 创建对象池并预分配指定数量的对象
     * 
     * @param initial_size 初始池大小（默认：10）
     * @param max_size 最大池大小（默认：1000）
     * @pre initial_size <= max_size
     * @post 对象池被初始化并包含 initial_size 个对象
     * @throw std::invalid_argument 如果 initial_size > max_size
     */
    explicit FqInfoBatchPool(size_t initial_size = 10, size_t max_size = 1000);
    
    /**
     * @brief 析构函数
     * @details 清理对象池中的所有对象
     * @post 所有对象被正确释放
     */
    ~FqInfoBatchPool();

    // 禁用拷贝和移动，确保线程安全
    FqInfoBatchPool(const FqInfoBatchPool&) = delete;
    auto operator=(const FqInfoBatchPool&) -> FqInfoBatchPool& = delete;
    FqInfoBatchPool(FqInfoBatchPool&&) = delete;
    auto operator=(FqInfoBatchPool&&) -> FqInfoBatchPool& = delete;

    /**
     * @brief 获取一个 FqInfoBatch 对象
     * @details 从对象池中获取一个可用的对象，如果池为空则创建新对象
     * 
     * @return std::unique_ptr<fq::fastq::FqInfoBatch> 指向 FqInfoBatch 对象的唯一指针
     * @post 返回的对象可供使用
     * @threadsafe 线程安全操作
     * @note 调用者负责在使用后将对象归还到池中
     */
    auto acquire() -> std::unique_ptr<fq::fastq::FqInfoBatch>;

    /**
     * @brief 释放一个 FqInfoBatch 对象回池中
     * @details 将使用完的对象归还到对象池中，以便后续重用
     * 
     * @param batch 要释放的批处理对象
     * @pre batch 必须是有效的对象指针
     * @post 对象被归还到池中，batch 指针被置为空
     * @threadsafe 线程安全操作
     * @note 如果池已满，对象将被销毁而不是放入池中
     */
    void release(std::unique_ptr<fq::fastq::FqInfoBatch> batch);

    /**
     * @brief 获取当前池大小
     * @details 获取对象池中当前可用的对象数量
     * 
     * @return size_t 池中可用对象的数量
     * @threadsafe 线程安全操作
     * @note 该值是瞬时的，可能随时变化
     */
    [[nodiscard]] auto pool_size() const noexcept -> size_t;

    /**
     * @brief 获取活跃对象数量
     * @details 获取当前已分配但未归还的对象数量
     * 
     * @return size_t 活跃对象的数量
     * @threadsafe 线程安全操作
     * @note 该值是瞬时的，可能随时变化
     */
    [[nodiscard]] auto active_count() const noexcept -> size_t;

    /**
     * @brief 收缩池大小
     * @details 减少池中的对象数量，释放未使用的内存
     * @post 池大小被调整为更合适的值
     * @threadsafe 线程安全操作
     * @note 该操作不会影响活跃对象
     */
    void shrink();

    /**
     * @brief 扩展池大小
     * @details 增加池中的对象数量，提高性能
     * 
     * @param count 要添加的对象数量
     * @pre count > 0
     * @post 池大小增加，但不超过最大限制
     * @threadsafe 线程安全操作
     * @note 如果池大小已达到最大限制，该操作可能不会添加所有请求的对象
     */
    void expand(size_t count);

    /**
     * @brief 预分配指定数量的对象
     * @details 预先分配指定数量的对象到池中，避免运行时分配开销
     * 
     * @param count 要预分配的对象数量
     * @pre count > 0
     * @post 池中至少包含 count 个对象
     * @threadsafe 线程安全操作
     * @note 该操作会阻塞直到所有对象分配完成
     */
    void preallocate(size_t count);

    /**
     * @brief 内存统计信息结构体
     * @details 记录对象池的内存使用和性能统计信息
     * 
     * @note 所有统计信息都是累计值，可用于性能分析
     */
    struct MemoryStats {
        size_t pool_size = 0;          ///< 当前池大小
        size_t active_count = 0;       ///< 活跃对象数量
        size_t total_allocated = 0;    ///< 总分配次数
        size_t total_freed = 0;        ///< 总释放次数
        size_t hit_count = 0;           ///< 命中次数（池中获取对象）
        size_t miss_count = 0;          ///< 未命中次数（创建新对象）
    };

    /**
     * @brief 获取内存使用统计
     * @details 获取对象池的详细统计信息
     * 
     * @return MemoryStats 统计信息结构体
     * @threadsafe 线程安全操作
     * @note 统计信息是瞬时的快照
     */
    [[nodiscard]] auto get_stats() const noexcept -> MemoryStats;

    /**
     * @brief 重置统计信息
     * @details 将所有统计计数器重置为零
     * @post 所有统计信息被重置
     * @threadsafe 线程安全操作
     */
    void reset_stats() noexcept;

private:
    mutable std::mutex m_mutex;                                             ///< 对象池互斥锁
    std::queue<std::unique_ptr<fq::fastq::FqInfoBatch>> m_pool;           ///< 对象池队列
    std::atomic<size_t> m_active_count{0};                                 ///< 活跃对象计数器
    size_t m_max_size;                                                     ///< 最大池大小
    
    // 统计信息
    mutable std::mutex m_stats_mutex;                                      ///< 统计信息互斥锁
    std::atomic<size_t> m_total_allocated{0};                              ///< 总分配次数
    std::atomic<size_t> m_total_freed{0};                                  ///< 总释放次数
    std::atomic<size_t> m_hit_count{0};                                    ///< 命中次数
    std::atomic<size_t> m_miss_count{0};                                   ///< 未命中次数

    /**
     * @brief 创建新的 FqInfoBatch 对象
     * @details 创建新的 FqInfoBatch 对象并更新统计信息
     * 
     * @return std::unique_ptr<fq::fastq::FqInfoBatch> 新创建的对象
     * @post 统计信息被更新
     * @note 该方法仅在池为空时调用
     */
    auto create_object() -> std::unique_ptr<fq::fastq::FqInfoBatch>;
};

/**
 * @brief 批处理内存管理器，统一管理所有内存对象的分配和释放
 * @details 该类提供了高级的内存管理功能，包括：
 *          - 对象池管理
 *          - 内存使用监控
 *          - 自动内存优化
 *          - 统计信息收集
 * 
 * @invariant 内存使用量不超过配置的最大限制
 * @warning 该类设计为单例模式，不建议创建多个实例
 */
class batch_memory_manager {
public:
    /**
     * @brief 配置结构体
     * @details 定义内存管理器的各种配置参数
     * 
     * @note 所有参数都有合理的默认值
     */
    struct Config {
        size_t max_memory_mb;                           ///< 最大内存使用量（MB）
        size_t initial_batch_pool_size;                ///< 初始批处理池大小
        size_t max_batch_pool_size;                    ///< 最大批处理池大小
        bool enable_auto_shrink;                       ///< 启用自动收缩
        std::chrono::seconds shrink_interval;         ///< 收缩间隔
        bool enable_stats;                             ///< 启用统计信息收集
        
        /**
         * @brief 默认构造函数
         * @details 设置合理的默认配置值
         */
        Config() : max_memory_mb(1024), 
                  initial_batch_pool_size(10), 
                  max_batch_pool_size(1000),
                  enable_auto_shrink(true),
                  shrink_interval(30),
                  enable_stats(true) {}
    };

    /**
     * @brief 构造函数
     * @details 使用指定配置创建内存管理器
     * 
     * @param config 配置参数（可选，使用默认值）
     * @post 内存管理器被初始化并准备使用
     * @throw std::invalid_argument 如果配置参数无效
     */
    explicit batch_memory_manager(const Config& config = Config{});
    
    /**
     * @brief 析构函数
     * @details 清理所有资源，包括对象池和后台线程
     * @post 所有资源被正确释放
     */
    ~batch_memory_manager();

    // 禁用拷贝和移动，确保线程安全
    batch_memory_manager(const batch_memory_manager&) = delete;
    auto operator=(const batch_memory_manager&) -> batch_memory_manager& = delete;
    batch_memory_manager(batch_memory_manager&&) = delete;
    auto operator=(batch_memory_manager&&) -> batch_memory_manager& = delete;

    /**
     * @brief 获取一个 FqInfoBatch 对象
     * @details 从内存管理器中获取一个可用的批处理对象
     * 
     * @return std::unique_ptr<fq::fastq::FqInfoBatch> 指向批处理对象的唯一指针
     * @post 返回的对象可供使用
     * @threadsafe 线程安全操作
     * @note 调用者负责在使用后将对象归还
     */
    [[nodiscard]] auto acquire_batch() -> std::unique_ptr<fq::fastq::FqInfoBatch>;

    /**
     * @brief 释放一个 FqInfoBatch 对象
     * @details 将使用完的批处理对象归还到内存管理器
     * 
     * @param batch 要释放的批处理对象
     * @pre batch 必须是有效的对象指针
     * @post 对象被归还到对象池中
     * @threadsafe 线程安全操作
     */
    void release_batch(std::unique_ptr<fq::fastq::FqInfoBatch> batch);

    /**
     * @brief 获取内存使用统计
     * @details 获取当前内存使用量（以字节为单位）
     * 
     * @return size_t 当前内存使用量
     * @threadsafe 线程安全操作
     * @note 该值是瞬时的，可能随时变化
     */
    [[nodiscard]] auto get_memory_usage() const noexcept -> size_t;

    /**
     * @brief 获取活跃对象数量
     * @details 获取当前活跃的批处理对象数量
     * 
     * @return size_t 活跃对象数量
     * @threadsafe 线程安全操作
     */
    [[nodiscard]] auto get_active_objects() const noexcept -> size_t;

    /**
     * @brief 获取批处理池统计信息
     * @details 获取批处理对象池的详细统计信息
     * 
     * @return FqInfoBatchPool::MemoryStats 统计信息结构体
     * @threadsafe 线程安全操作
     */
    [[nodiscard]] auto get_batch_pool_stats() const -> FqInfoBatchPool::MemoryStats;

    /**
     * @brief 优化内存使用
     * @details 执行内存优化操作，包括收缩对象池等
     * @post 内存使用被优化
     * @threadsafe 线程安全操作
     */
    void optimize();

    /**
     * @brief 获取配置信息
     * @details 获取当前的配置参数
     * 
     * @return const Config& 配置信息的常量引用
     * @threadsafe 线程安全操作
     */
    [[nodiscard]] auto get_config() const noexcept -> const Config&;

    /**
     * @brief 更新配置
     * @details 动态更新内存管理器的配置参数
     * 
     * @param config 新的配置参数
     * @pre config 必须包含有效的配置参数
     * @post 配置被更新，某些参数可能立即生效
     * @threadsafe 线程安全操作
     * @note 某些配置更改可能需要重启才能生效
     */
    void update_config(const Config& config);

private:
    Config m_config;                                                  ///< 配置参数
    std::unique_ptr<FqInfoBatchPool> m_batch_pool;                   ///< 批处理对象池
    
    // 自动收缩线程
    std::thread m_shrink_thread;                                      ///< 自动收缩工作线程
    std::atomic<bool> m_stop_shrinking{false};                        ///< 停止收缩标志
    
    /**
     * @brief 自动收缩工作线程
     * @details 定期执行内存优化操作的后台线程
     */
    void shrink_worker();
    
    /**
     * @brief 启动自动收缩线程
     * @details 创建并启动后台收缩线程
     * @post 收缩线程开始运行
     */
    void start_shrink_thread();
    
    /**
     * @brief 停止自动收缩线程
     * @details 停止并清理后台收缩线程
     * @post 收缩线程停止运行
     */
    void stop_shrink_thread();
};

/**
 * @brief 全局内存管理器访问器
 * @details 获取全局内存管理器实例的单例访问器
 * 
 * @return std::shared_ptr<batch_memory_manager> 全局内存管理器的共享指针
 * @note 如果未初始化，会自动创建默认配置的实例
 */
auto global_memory_manager() -> std::shared_ptr<batch_memory_manager>;

/**
 * @brief 初始化全局内存管理器
 * @details 使用指定配置初始化全局内存管理器
 * 
 * @param config 配置参数（可选，使用默认值）
 * @post 全局内存管理器被初始化
 * @warning 如果已存在实例，会被替换
 */
void init_global_memory_manager(const batch_memory_manager::Config& config = batch_memory_manager::Config{});

/**
 * @brief 清理全局内存管理器
 * @details 销毁全局内存管理器实例，释放所有资源
 * @post 全局内存管理器被销毁
 * @warning 清理后不能再使用 global_memory_manager()
 */
void cleanup_global_memory_manager();

} // namespace fq::memory

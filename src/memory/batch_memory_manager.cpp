#include "batch_memory_manager.h"
#include <iostream>
#include <thread>
#include <algorithm>

namespace fq::memory {

//==============================================================================
// FqInfoBatchPool Implementation
//==============================================================================

FqInfoBatchPool::FqInfoBatchPool(size_t initial_size, size_t max_size)
    : m_max_size(max_size) {
    
    preallocate(initial_size);
}

FqInfoBatchPool::~FqInfoBatchPool() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 清理池中的所有对象
    while (!m_pool.empty()) {
        m_pool.pop();
    }
}

auto FqInfoBatchPool::acquire() -> std::unique_ptr<fq::fastq::FqInfoBatch> {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_pool.empty()) {
            auto batch = std::move(m_pool.front());
            m_pool.pop();
            m_active_count++;
            m_hit_count++;
            return batch;
        }
    }
    
    // 池为空，创建新对象
    m_miss_count++;
    m_active_count++;
    m_total_allocated++;
    return create_object();
}

void FqInfoBatchPool::release(std::unique_ptr<fq::fastq::FqInfoBatch> batch) {
    if (!batch) {
        return;
    }
    
    // 清理批处理对象
    batch->clear();
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_pool.size() < m_max_size) {
            m_pool.push(std::move(batch));
            m_active_count--;
            m_total_freed++;
            return;
        }
    }
    
    // 池已满，直接删除对象
    m_active_count--;
    m_total_freed++;
}

auto FqInfoBatchPool::pool_size() const noexcept -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pool.size();
}

auto FqInfoBatchPool::active_count() const noexcept -> size_t {
    return m_active_count.load();
}

void FqInfoBatchPool::shrink() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 保留一些基本对象，避免频繁分配
    const size_t min_keep = std::max(size_t(5), m_max_size / 10);
    size_t current_size = m_pool.size();
    
    if (current_size > min_keep) {
        size_t to_remove = current_size - min_keep;
        for (size_t i = 0; i < to_remove; ++i) {
            m_pool.pop();
        }
    }
}

void FqInfoBatchPool::expand(size_t count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t current_size = m_pool.size();
    size_t can_add = std::min(count, m_max_size - current_size);
    
    for (size_t i = 0; i < can_add; ++i) {
        m_pool.push(create_object());
    }
    // 统计预扩容的分配次数
    m_total_allocated += can_add;
}

void FqInfoBatchPool::preallocate(size_t count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t to_allocate = std::min(count, m_max_size);
    
    for (size_t i = 0; i < to_allocate; ++i) {
        m_pool.push(create_object());
    }
    
    m_total_allocated += to_allocate;
}

auto FqInfoBatchPool::get_stats() const noexcept -> FqInfoBatchPool::MemoryStats {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    return {
        pool_size(),
        active_count(),
        m_total_allocated.load(),
        m_total_freed.load(),
        m_hit_count.load(),
        m_miss_count.load()
    };
}

void FqInfoBatchPool::reset_stats() noexcept {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    m_total_allocated = 0;
    m_total_freed = 0;
    m_hit_count = 0;
    m_miss_count = 0;
}

auto FqInfoBatchPool::create_object() -> std::unique_ptr<fq::fastq::FqInfoBatch> {
    return std::make_unique<fq::fastq::FqInfoBatch>();
}

//==============================================================================
// BatchMemoryManager Implementation
//==============================================================================

BatchMemoryManager::BatchMemoryManager(const Config& config)
    : m_config(config) {
    
    // 创建批处理池
    m_batch_pool = std::make_unique<FqInfoBatchPool>(
        config.initial_batch_pool_size,
        config.max_batch_pool_size
    );
    
    // 启动自动收缩线程
    if (config.enable_auto_shrink) {
        start_shrink_thread();
    }
}

BatchMemoryManager::~BatchMemoryManager() {
    stop_shrink_thread();
}

auto BatchMemoryManager::acquire_batch() -> std::unique_ptr<fq::fastq::FqInfoBatch> {
    return m_batch_pool->acquire();
}

void BatchMemoryManager::release_batch(std::unique_ptr<fq::fastq::FqInfoBatch> batch) {
    m_batch_pool->release(std::move(batch));
}

auto BatchMemoryManager::get_memory_usage() const noexcept -> size_t {
    // 简化的内存使用计算
    size_t pool_size = m_batch_pool->pool_size();
    size_t active_count = m_batch_pool->active_count();
    
    // 估算每个FqInfoBatch的平均内存使用
    constexpr size_t estimated_batch_size = 1024 * 1024; // 1MB per batch
    // 按字节返回（与头文件说明一致）
    return (pool_size + active_count) * estimated_batch_size; // bytes
}

auto BatchMemoryManager::get_active_objects() const noexcept -> size_t {
    return m_batch_pool->active_count();
}

auto BatchMemoryManager::get_batch_pool_stats() const -> FqInfoBatchPool::MemoryStats {
    return m_batch_pool->get_stats();
}

auto BatchMemoryManager::get_config() const noexcept -> const BatchMemoryManager::Config& {
    return m_config;
}

void BatchMemoryManager::update_config(const Config& config) {
    bool was_auto_shrink = m_config.enable_auto_shrink;
    m_config = config;
    
    // 更新池配置
    if (config.enable_auto_shrink && !was_auto_shrink) {
        start_shrink_thread();
    } else if (!config.enable_auto_shrink && was_auto_shrink) {
        stop_shrink_thread();
    }
}

void BatchMemoryManager::optimize() {
    // 执行内存优化
    m_batch_pool->shrink();
}

void BatchMemoryManager::shrink_worker() {
    while (!m_stop_shrinking) {
        std::this_thread::sleep_for(m_config.shrink_interval);
        
        if (m_stop_shrinking) {
            break;
        }
        
        try {
            optimize();
        } catch (const std::exception& e) {
            std::cerr << "Error in memory manager shrink worker: " << e.what() << std::endl;
        }
    }
}

void BatchMemoryManager::start_shrink_thread() {
    if (!m_shrink_thread.joinable()) {
        // 确保标志位复位，以便线程可被正确重启
        m_stop_shrinking = false;
        m_shrink_thread = std::thread(&BatchMemoryManager::shrink_worker, this);
    }
}

void BatchMemoryManager::stop_shrink_thread() {
    if (m_shrink_thread.joinable()) {
        m_stop_shrinking = true;
        m_shrink_thread.join();
    }
}

//==============================================================================
// Global Memory Manager Functions
//==============================================================================

namespace {
    std::shared_ptr<BatchMemoryManager> g_global_memory_manager;
    std::mutex g_memory_manager_mutex;
}

auto global_memory_manager() -> std::shared_ptr<BatchMemoryManager> {
    std::lock_guard<std::mutex> lock(g_memory_manager_mutex);
    return g_global_memory_manager;
}

void init_global_memory_manager(const BatchMemoryManager::Config& config) {
    std::lock_guard<std::mutex> lock(g_memory_manager_mutex);
    
    if (!g_global_memory_manager) {
        g_global_memory_manager = std::make_shared<BatchMemoryManager>(config);
    }
}

void cleanup_global_memory_manager() {
    std::lock_guard<std::mutex> lock(g_memory_manager_mutex);
    g_global_memory_manager.reset();
}

} // namespace fq::memory

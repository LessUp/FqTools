#pragma once

/**
 * @file conveyor.h
 * @brief 线程安全的数据传输带组件
 * @details 该组件实现了生产者-消费者模式，用于在线程间高效传递数据块
 * 
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#include "spdlog/spdlog.h"
#include "tbb/concurrent_queue.h"

#include <thread>
#include <functional>
#include <memory>

namespace fq::common {

/**
 * @brief 线程安全的数据传输带，用于在生产者和消费者线程间传递数据
 * @details 该类管理两个队列：一个'空闲'队列保存预分配的空数据块，
 *          一个'填充'队列保存已填充数据并准备处理的数据块
 * 
 * 该组件基于 Intel TBB 的高性能 concurrent_queue 构建，确保线程安全和效率
 * 
 * @tparam D 要管理的数据块类型（例如：指向数据结构的指针）
 * @invariant 空闲队列和填充队列的元素总数应保持恒定（对象池模式）
 * @warning 析构时会清理所有剩余数据块，请确保数据处理完成
 */
template <typename D>
class conveyor {
public:
    /**
     * @brief 构造函数，创建 conveyor 并预分配对象池
     * @details 在后台线程中预分配指定数量的数据块，提高运行时性能
     * 
     * @param malloc_func 用于分配新数据块的函数
     * @param size 要预分配的数据块数量
     * @param delete_func 用于释放数据块的函数（默认为 delete 操作）
     * @pre malloc_func 必须返回有效的数据块
     * @post 对象池中包含指定数量的空闲数据块
     * @note 初始化在后台线程中进行，不会阻塞构造函数
     */
    conveyor(
        std::function<D()> malloc_func, 
        size_t size, 
        std::function<void(D&)> delete_func = [](D& data) { delete data; })
        : m_malloc_func(malloc_func), 
          m_delete_func(delete_func)
    {
        // 在后台线程中预分配对象池，避免阻塞主线程
        m_init_thread = std::make_unique<std::thread>([this, size]() {
            for(size_t i = 0; i < size; ++i) {
                D data = this->m_malloc_func();
                enqueueDataToFree(data);
            }
        });
    }

    /**
     * @brief 析构函数，清理所有资源
     * @details 等待初始化线程完成，然后清理空闲队列和填充队列中的所有数据块
     * @warning 如果填充队列中仍有数据块，会发出警告并清理
     * @note 确保所有数据处理完成后再销毁对象
     */
    ~conveyor() {
        if (m_init_thread && m_init_thread->joinable()) {
            m_init_thread->join();
        }

        // 清理空闲队列中的所有数据块
        D data;
        while(dequeueDataFromFree(data)) {
            m_delete_func(data);
        }

        // 检查并清理填充队列中的剩余数据块
        if (getFill() > 0) {
            spdlog::warn("conveyor has {} items in fill_queue on destruction.", getFill());
            while(dequeueDataFromFill(data)) {
                m_delete_func(data);
            }
        }
    }

    // 禁用拷贝构造和赋值操作，确保线程安全
    conveyor(const conveyor&) = delete;
    auto operator=(const conveyor&) -> conveyor& = delete;

    /**
     * @brief 将数据块放回空闲池中
     * @details 生产者使用此方法将处理完的数据块归还到空闲队列
     * 
     * @param data 要归还的数据块引用
     * @pre data 必须是有效的数据块
     * @post 数据块被添加到空闲队列中
     * @threadsafe 线程安全操作
     */
    void enqueueDataToFree(D& data) { 
        free_queue.push(data); 
    }

    /**
     * @brief 尝试从空闲池中获取一个空数据块
     * @details 生产者使用此方法获取空数据块进行填充
     * 
     * @param data 用于存储获取的数据块的引用
     * @return 成功获取返回 true，队列为空返回 false
     * @pre data 必须是有效的引用
     * @post 如果成功，data 包含一个空数据块
     * @threadsafe 线程安全操作
     */
    auto dequeueDataFromFree(D& data) -> bool { 
        return free_queue.try_pop(data);
    }

    /**
     * @brief 获取空闲队列中的数据块数量（近似值）
     * @details 返回的值是近似的，因为在多线程环境下队列状态可能随时变化
     * 
     * @return 空闲队列中数据块的近似数量
     * @note 该值仅用于监控和调试，不应用于业务逻辑判断
     * @threadsafe 线程安全操作
     */
    auto getFree() const -> size_t { 
        return free_queue.unsafe_size(); 
    }

    /**
     * @brief 将填充好数据的数据块放入处理队列
     * @details 生产者使用此方法将填充好数据的数据块提交给消费者处理
     * 
     * @param data 要提交的数据块引用
     * @pre data 必须包含有效数据
     * @post 数据块被添加到填充队列中
     * @threadsafe 线程安全操作
     */
    void enqueueDataToFill(D& data) { 
        fill_queue.push(data); 
    }

    /**
     * @brief 尝试从处理队列中获取一个填充好的数据块
     * @details 消费者使用此方法获取待处理的数据块
     * 
     * @param data 用于存储获取的数据块的引用
     * @return 成功获取返回 true，队列为空返回 false
     * @pre data 必须是有效的引用
     * @post 如果成功，data 包含一个填充好数据的数据块
     * @threadsafe 线程安全操作
     */
    auto dequeueDataFromFill(D& data) -> bool { 
        return fill_queue.try_pop(data);
    }

    /**
     * @brief 获取填充队列中的数据块数量（近似值）
     * @details 返回的值是近似的，因为在多线程环境下队列状态可能随时变化
     * 
     * @return 填充队列中数据块的近似数量
     * @note 该值仅用于监控和调试，不应用于业务逻辑判断
     * @threadsafe 线程安全操作
     */
    auto getFill() const -> size_t { 
        return fill_queue.unsafe_size(); 
    }

private:
    std::function<D()> m_malloc_func;              ///< 内存分配函数
    std::function<void(D&)> m_delete_func;          ///< 内存释放函数
    std::unique_ptr<std::thread> m_init_thread;    ///< 初始化线程

    tbb::concurrent_queue<D> free_queue;            ///< 空闲数据块队列
    tbb::concurrent_queue<D> fill_queue;            ///< 填充数据块队列
};

} // namespace fq::common

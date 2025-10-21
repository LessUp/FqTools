/**
 * @file i_read_processor.h
 * @brief 读取处理器接口定义
 * @details 该文件定义了读取处理器的抽象接口，包括数据修改器和数据过滤器
 *
 * @author FastQTools Team
 * @date 2024
 * @version 1.0
 *
 * @copyright Copyright (c) 2024 FastQTools
 * @license MIT License
 */

#pragma once
#include "core_legacy/core.h"
#include <string>

namespace fq::processing
{

/**
 * @brief 读取数据修改器接口
 * @details 该接口定义了对FASTQ读取数据进行修改的操作，例如质量修剪、长度修剪等
 */
class IReadMutator
{
public:
    virtual ~IReadMutator() = default;

    /**
     * @brief 处理单个FASTQ读取数据
     * @details 对输入的FASTQ读取数据进行修改操作
     *
     * @param read 要处理的FASTQ读取数据
     * @return 处理成功返回true，失败返回false
     */
    virtual auto process(fq::fastq::FqInfo &read) -> bool = 0;
};

/**
 * @brief 读取数据过滤器接口
 * @details 该接口定义了对FASTQ读取数据进行过滤判断的操作，用于决定是否保留该读取数据
 */
class IReadPredicate
{
public:
    virtual ~IReadPredicate() = default;

    /**
     * @brief 评估单个FASTQ读取数据是否满足条件
     * @details 对输入的FASTQ读取数据进行评估，判断是否满足保留条件
     *
     * @param read 要评估的FASTQ读取数据
     * @return 满足条件返回true，不满足返回false
     */
    virtual auto evaluate(const fq::fastq::FqInfo &read) const -> bool = 0;
};

} // namespace fq::processing

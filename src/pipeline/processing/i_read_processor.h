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
#include "fqtools/pipeline/read_mutator_interface.h"
#include "fqtools/pipeline/read_predicate_interface.h"

namespace fq::processing {

// 兼容别名：保留旧名称用于现有实现的继承与使用
using IReadMutator = ReadMutatorInterface;
using IReadPredicate = ReadPredicateInterface;

} // namespace fq::processing

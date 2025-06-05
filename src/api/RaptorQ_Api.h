/*
 * Copyright (c) 2016-2018, Luca Fulchir<luca@fulchir.it>, All rights reserved.
 *
 * This file is part of "libRaptorQ".
 *
 * libRaptorQ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * libRaptorQ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and a copy of the GNU Lesser General Public License
 * along with libRaptorQ.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 获取可用块大小列表
 * 
 * @param sizes 用于存储块大小的数组
 * @param size 数组的大小
 * @return 实际填充的元素数量，如果sizes为NULL，则返回可用块大小的总数
 */
uint32_t get_usable_block_sizes(uint16_t* sizes, uint32_t size);

/**
 * RaptorQ编码函数
 * 
 * @param symbol_size 符号大小（字节）
 * @param symbols 块大小（符号数量）
 * @param repair 修复符号数量
 * @param input_data 输入数据缓冲区
 * @param input_size 输入数据大小
 * @param output_data 输出数据缓冲区（调用者负责分配足够空间）
 * @param output_size 输出缓冲区大小
 * @param actual_output_size 实际输出数据大小（返回值）
 * @return 1表示成功，0表示失败
 */
int raptorq_encode(int64_t symbol_size, uint16_t symbols, uint32_t repair,
                   const uint8_t* input_data, size_t input_size,
                   uint8_t* output_data, size_t output_size,
                   size_t* actual_output_size);

/**
 * RaptorQ解码函数
 * 
 * @param bytes 原始数据大小（字节）
 * @param symbols 块大小（符号数量）
 * @param symbol_size 符号大小（字节）
 * @param input_data 输入编码数据缓冲区
 * @param input_size 输入数据大小
 * @param output_data 输出解码数据缓冲区（调用者负责分配足够空间）
 * @param output_size 输出缓冲区大小
 * @param actual_output_size 实际输出数据大小（返回值）
 * @return 1表示成功，0表示失败
 */
int raptorq_decode(size_t bytes, uint16_t symbols, int64_t symbol_size,
                   const uint8_t* input_data, size_t input_size,
                   uint8_t* output_data, size_t output_size,
                   size_t* actual_output_size);

#ifdef __cplusplus
}
#endif
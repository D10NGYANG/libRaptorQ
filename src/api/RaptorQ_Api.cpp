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

#include "RaptorQ_Api.h"
#include "RaptorQ/RaptorQ_v1_hdr.hpp"
#include <vector>
#include <cstring>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

/**
 * 获取可用块大小列表
 * 
 * @param sizes 用于存储块大小的数组
 * @param size 数组的大小
 * @return 实际填充的元素数量，如果sizes为NULL，则返回可用块大小的总数
 */
uint32_t get_usable_block_sizes(uint16_t* sizes, uint32_t size) {
    // 获取所有可用块大小
    uint32_t total_size = static_cast<uint32_t>(RaptorQ__v1::blocks->size());
    
    // 如果sizes为NULL，只返回可用块大小的总数
    if (sizes == nullptr) {
        return total_size;
    }
    
    // 计算实际要填充的元素数量
    uint32_t fill_size = (size < total_size) ? size : total_size;
    
    // 填充数组
    for (uint32_t idx = 0; idx < fill_size; ++idx) {
        sizes[idx] = static_cast<uint16_t>((*RaptorQ__v1::blocks)[idx]);
    }
    
    return fill_size;
}

/**
 * RaptorQ编码函数
 */
int raptorq_encode(int64_t symbol_size,
                   uint16_t symbols,
                   uint32_t repair,
                   const uint8_t* input_data,
                   size_t input_size,
                   uint8_t* output_data,
                   size_t output_size,
                   size_t* actual_output_size)
{
    if (!input_data || !output_data || !actual_output_size) {
        return 0; // 参数错误
    }
    
    *actual_output_size = 0;
    
    // 定义迭代器类型
    using iter_8 = std::vector<uint8_t>::iterator;
    
    // 创建输入和输出缓冲区
    std::vector<uint8_t> buf(static_cast<size_t>(symbol_size), 0);
    std::vector<uint8_t> block_buf;
    block_buf.reserve(static_cast<size_t>(symbols) * static_cast<size_t>(symbol_size));
    
    // 创建编码器
    RaptorQ__v1::Encoder<iter_8, iter_8> encoder(static_cast<RaptorQ__v1::Block_Size>(symbols),
                                                  static_cast<size_t>(symbol_size));
    auto future = encoder.precompute();
    RaptorQ__v1::Error enc_status = RaptorQ__v1::Error::INITIALIZATION;
    
    uint32_t sym_num = 0;
    uint32_t block_num = 0;
    size_t input_offset = 0;
    size_t output_offset = 0;
    
    while (input_offset < input_size) {
        // 清空缓冲区并填充数据
        buf.clear();
        buf.insert(buf.begin(), static_cast<size_t>(symbol_size), 0);
        
        // 从输入数据读取
        size_t bytes_to_read = std::min(static_cast<size_t>(symbol_size), input_size - input_offset);
        if (bytes_to_read > 0) {
            std::memcpy(buf.data(), input_data + input_offset, bytes_to_read);
            input_offset += bytes_to_read;
            
            // 检查输出缓冲区空间
            size_t needed_space = sizeof(block_num) + sizeof(sym_num) + symbol_size;
            if (output_offset + needed_space > output_size) {
                return 0; // 输出缓冲区空间不足
            }
            
            // 写入块号、符号号和数据
            std::memcpy(output_data + output_offset, &block_num, sizeof(block_num));
            output_offset += sizeof(block_num);
            std::memcpy(output_data + output_offset, &sym_num, sizeof(sym_num));
            output_offset += sizeof(sym_num);
            std::memcpy(output_data + output_offset, buf.data(), symbol_size);
            output_offset += symbol_size;
            
            block_buf.insert(block_buf.end(), buf.begin(), buf.end());
            ++sym_num;
        }
        
        // 检查是否到达输入末尾或块已满
        if (input_offset >= input_size || sym_num == static_cast<uint16_t>(symbols)) {
            // 如果到达输入末尾，设置符号数为块大小
            if (input_offset >= input_size) {
                sym_num = static_cast<uint16_t>(symbols);
            }
            
            if (sym_num == static_cast<uint16_t>(symbols)) {
                // 将数据提供给编码器
                size_t ret = encoder.set_data(block_buf.begin(), block_buf.end());
                if (ret != block_buf.size()) {
                    return 0; // 无法添加块数据到编码器
                }
                
                // 等待预计算完成
                if (future.valid()) {
                    future.wait();
                    enc_status = future.get();
                }
                if (enc_status != RaptorQ__v1::Error::NONE) {
                    return 0; // 编码器错误
                }
                
                // 生成修复符号
                std::vector<uint8_t> rep(static_cast<size_t>(symbol_size), 0);
                for (uint32_t rep_id = sym_num; rep_id < (static_cast<size_t>(symbols) + repair); ++rep_id) {
                    auto rep_start = rep.begin();
                    auto rep_length = encoder.encode(rep_start, rep.end(), rep_id);
                    
                    if (rep_length != static_cast<size_t>(symbol_size)) {
                        return 0; // 修复符号大小错误
                    }
                    
                    // 检查输出缓冲区空间
                    size_t needed_space = sizeof(block_num) + sizeof(rep_id) + symbol_size;
                    if (output_offset + needed_space > output_size) {
                        return 0; // 输出缓冲区空间不足
                    }
                    
                    // 写入修复符号
                    std::memcpy(output_data + output_offset, &block_num, sizeof(block_num));
                    output_offset += sizeof(block_num);
                    std::memcpy(output_data + output_offset, &rep_id, sizeof(rep_id));
                    output_offset += sizeof(rep_id);
                    std::memcpy(output_data + output_offset, rep.data(), symbol_size);
                    output_offset += symbol_size;
                }
                
                // 如果处理完所有输入数据，退出
                if (input_offset >= input_size) {
                    break;
                }
                
                // 准备下一个块
                encoder.clear_data();
                block_buf.clear();
                ++block_num;
                sym_num = 0;
            }
        }
    }
    
    *actual_output_size = output_offset;
    return 1;
}

/**
 * RaptorQ解码函数
 */
int raptorq_decode(size_t bytes, uint16_t symbols, int64_t symbol_size,
                   const uint8_t* input_data, size_t input_size,
                   uint8_t* output_data, size_t output_size,
                   size_t* actual_output_size) {
    
    // 参数验证
    if (input_data == nullptr || output_data == nullptr || actual_output_size == nullptr) {
        return 0;
    }
    
    if (bytes == 0 || symbols == 0 || symbol_size == 0 || input_size == 0 || output_size == 0) {
        return 0;
    }
    
    // 检查输出缓冲区大小
    if (output_size < bytes) {
        return 0;
    }
    
    // 验证块大小是否有效
    RaptorQ__v1::Block_Size block_size = static_cast<RaptorQ__v1::Block_Size>(symbols);
    bool valid_block_size = false;
    for (const auto& size : *RaptorQ__v1::blocks) {
        if (size == block_size) {
            valid_block_size = true;
            break;
        }
    }
    if (!valid_block_size) {
        return 0;
    }
    
    using iter_8 = std::vector<uint8_t>::iterator;
    using Dec = RaptorQ__v1::Decoder<iter_8, iter_8>;
    
    std::vector<uint8_t> buf(static_cast<size_t>(symbol_size));
    std::map<size_t, std::unique_ptr<Dec>> decoders;
    size_t bytes_left = bytes;
    size_t last_block_bytes = 0;
    size_t input_offset = 0;
    size_t output_offset = 0;
    
    // 解析输入数据
    while (input_offset < input_size) {
        // 读取块号
        if (input_offset + sizeof(uint32_t) > input_size) {
            return 0; // 数据不足
        }
        uint32_t block_number;
        std::memcpy(&block_number, input_data + input_offset, sizeof(block_number));
        input_offset += sizeof(block_number);
        
        // 读取符号号
        if (input_offset + sizeof(uint32_t) > input_size) {
            return 0; // 数据不足
        }
        uint32_t symbol_number;
        std::memcpy(&symbol_number, input_data + input_offset, sizeof(symbol_number));
        input_offset += sizeof(symbol_number);
        
        // 读取符号数据
        if (input_offset + symbol_size > input_size) {
            return 0; // 数据不足
        }
        buf.clear();
        buf.resize(static_cast<size_t>(symbol_size));
        std::memcpy(buf.data(), input_data + input_offset, symbol_size);
        input_offset += symbol_size;
        
        // 查找或创建解码器
        auto dec_it = decoders.find(block_number);
        if (dec_it == decoders.end()) {
            // 创建新的解码器
            if (bytes_left == 0) {
                return 0; // 发现额外的块
            }
            last_block_bytes = std::min(bytes_left,
                                      static_cast<size_t>(static_cast<int64_t>(symbols) * symbol_size));
            bytes_left -= last_block_bytes;
            
            bool success;
            std::tie(dec_it, success) = decoders.emplace(std::make_pair(
                block_number,
                std::unique_ptr<Dec>(
                    new Dec(block_size, static_cast<size_t>(symbol_size),
                           Dec::Report::PARTIAL_FROM_BEGINNING))));
            if (!success) {
                return 0; // 无法添加解码器
            }
        }
        
        auto dec = dec_it->second.get();
        if (dec == nullptr) {
            continue; // 已解码并释放的块
        }
        
        // 添加符号到解码器
        auto symbol_start = buf.begin();
        auto err = dec->add_symbol(symbol_start, buf.end(), symbol_number);
        if (err != RaptorQ__v1::Error::NONE && err != RaptorQ__v1::Error::NOT_NEEDED) {
            return 0; // 添加符号错误
        }
    }
    
    // 检查最后一个块是否需要填充
    if (!decoders.empty()) {
        auto last_dec_it = decoders.rbegin();
        auto last_dec = last_dec_it->second.get();
        if (last_dec != nullptr && last_block_bytes != static_cast<size_t>(static_cast<int64_t>(symbols) * symbol_size)) {
            // 最后一个块没有填满，需要添加零填充符号
            std::vector<uint8_t> padding_buf(static_cast<size_t>(symbol_size), 0);
            uint16_t last_symbol = static_cast<uint16_t>(std::ceil(last_block_bytes / static_cast<float>(symbol_size)));
            
            // 添加足够的零填充符号以满足解码器需求
            for (uint16_t idx = 0; idx < last_dec->needed_symbols(); ++idx) {
                auto buf_start = padding_buf.begin();
                last_dec->add_symbol(buf_start, padding_buf.end(), last_symbol + idx);
            }
        }
    }
    
    // 解码所有块并输出数据
    size_t current_block = 0;
    bytes_left = bytes;
    
    while (bytes_left > 0) {
        auto dec_it = decoders.find(current_block);
        if (dec_it == decoders.end()) {
            return 0; // 找不到解码器
        }
        
        auto dec = dec_it->second.get();
        if (dec == nullptr) {
            return 0; // 解码器为空
        }
        
        // 等待解码完成
        auto pair = dec->wait_sync();
        if (pair.error != RaptorQ__v1::Error::NONE) {
            return 0; // 解码错误
        }
        
        // 输出解码的符号
        size_t sym_size = dec->symbol_size();
        std::vector<uint8_t> buffer(sym_size);
        
        for (uint16_t sym_idx = 0; sym_idx < dec->symbols(); ++sym_idx) {
            buffer.clear();
            buffer.insert(buffer.begin(), sym_size, 0);
            auto buf_start = buffer.begin();
            auto to_write = dec->decode_symbol(buf_start, buffer.end(), sym_idx);
            if (to_write != RaptorQ__v1::Error::NONE) {
                return 0; // 解码符号错误
            }
            
            size_t writes_left = std::min(bytes_left, static_cast<size_t>(sym_size));
            if (output_offset + writes_left > output_size) {
                return 0; // 输出缓冲区空间不足
            }
            
            std::memcpy(output_data + output_offset, buffer.data(), writes_left);
            output_offset += writes_left;
            bytes_left -= writes_left;
            
            if (bytes_left == 0) {
                break; // 已输出所有数据
            }
        }
        
        // 释放解码器并移动到下一个块
        dec_it->second.reset(nullptr);
        ++current_block;
    }
    
    *actual_output_size = output_offset;
    return 1;
}
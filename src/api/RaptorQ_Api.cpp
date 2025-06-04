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
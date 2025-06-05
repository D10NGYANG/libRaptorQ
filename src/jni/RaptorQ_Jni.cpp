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

#include "include/jni.h"
#include "../api/RaptorQ_Api.h"
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_hailiao_util_KTRaptorQ
 * Method:    getUsableBlockSizes
 * Signature: ([II)I
 */
JNIEXPORT jint JNICALL Java_com_hailiao_util_KTRaptorQ_getUsableBlockSizes
  (JNIEnv *env, jclass clazz, jintArray sizes, jint size)
{
    if (sizes == NULL) {
        // 如果sizes为NULL，返回可用块大小的总数
        return (jint)get_usable_block_sizes(NULL, 0);
    }
    
    // 获取数组长度
    jsize array_len = env->GetArrayLength(sizes);
    if (array_len < size) {
        size = array_len;
    }
    
    // 分配临时缓冲区
    uint16_t* temp_sizes = new uint16_t[size];
    
    // 调用原始函数
    uint32_t result = get_usable_block_sizes(temp_sizes, (uint32_t)size);
    
    // 将结果复制到Java数组
    jint* java_sizes = new jint[size];
    for (int i = 0; i < size && i < result; i++) {
        java_sizes[i] = (jint)temp_sizes[i];
    }
    
    env->SetIntArrayRegion(sizes, 0, size, java_sizes);
    
    // 清理内存
    delete[] temp_sizes;
    delete[] java_sizes;
    
    return (jint)result;
}

/*
 * Class:     com_hailiao_util_KTRaptorQ
 * Method:    encode
 * Signature: (III[BI[BI[I)I
 */
JNIEXPORT jint JNICALL Java_com_hailiao_util_KTRaptorQ_encode
  (JNIEnv *env, jclass clazz, jint symbol_size, jint symbols, jint repair,
   jbyteArray input_data, jint input_size, jbyteArray output_data, jint output_size, jintArray actual_output_size)
{
    // 获取输入数据
    jbyte* input_bytes = env->GetByteArrayElements(input_data, NULL);
    if (input_bytes == NULL) {
        return 0;
    }
    
    // 获取输出缓冲区
    jbyte* output_bytes = env->GetByteArrayElements(output_data, NULL);
    if (output_bytes == NULL) {
        env->ReleaseByteArrayElements(input_data, input_bytes, JNI_ABORT);
        return 0;
    }
    
    // 调用原始编码函数
    size_t actual_size = 0;
    int result = raptorq_encode(
        (int64_t)symbol_size,
        (uint16_t)symbols,
        (uint32_t)repair,
        (const uint8_t*)input_bytes,
        (size_t)input_size,
        (uint8_t*)output_bytes,
        (size_t)output_size,
        &actual_size
    );
    
    // 设置实际输出大小
    if (actual_output_size != NULL) {
        jint actual_size_int = (jint)actual_size;
        env->SetIntArrayRegion(actual_output_size, 0, 1, &actual_size_int);
    }
    
    // 释放数组
    env->ReleaseByteArrayElements(input_data, input_bytes, JNI_ABORT);
    env->ReleaseByteArrayElements(output_data, output_bytes, 0);
    
    return (jint)result;
}

/*
 * Class:     com_hailiao_util_KTRaptorQ
 * Method:    decode
 * Signature: (III[BI[BI[I)I
 */
JNIEXPORT jint JNICALL Java_com_hailiao_util_KTRaptorQ_decode
  (JNIEnv *env, jclass clazz, jint bytes, jint symbols, jint symbol_size,
   jbyteArray input_data, jint input_size, jbyteArray output_data, jint output_size, jintArray actual_output_size)
{
    // 获取输入数据
    jbyte* input_bytes = env->GetByteArrayElements(input_data, NULL);
    if (input_bytes == NULL) {
        return 0;
    }
    
    // 获取输出缓冲区
    jbyte* output_bytes = env->GetByteArrayElements(output_data, NULL);
    if (output_bytes == NULL) {
        env->ReleaseByteArrayElements(input_data, input_bytes, JNI_ABORT);
        return 0;
    }
    
    // 调用原始解码函数
    size_t actual_size = 0;
    int result = raptorq_decode(
        (size_t)bytes,
        (uint16_t)symbols,
        (int64_t)symbol_size,
        (const uint8_t*)input_bytes,
        (size_t)input_size,
        (uint8_t*)output_bytes,
        (size_t)output_size,
        &actual_size
    );
    
    // 设置实际输出大小
    if (actual_output_size != NULL) {
        jint actual_size_int = (jint)actual_size;
        env->SetIntArrayRegion(actual_output_size, 0, 1, &actual_size_int);
    }
    
    // 释放数组
    env->ReleaseByteArrayElements(input_data, input_bytes, JNI_ABORT);
    env->ReleaseByteArrayElements(output_data, output_bytes, 0);
    
    return (jint)result;
}

#ifdef __cplusplus
}
#endif
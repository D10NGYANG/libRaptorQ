package com.hailiao.util;

/**
 * RaptorQ编解码库的Java接口
 * 提供RaptorQ前向纠错编码和解码功能
 */
public class KTRaptorQ {
    
    // 加载本地库
    static {
        try {
            // 尝试加载系统库
            System.loadLibrary("RaptorQ");
        } catch (UnsatisfiedLinkError e) {
            try {
                // 如果系统库加载失败，尝试加载当前目录下的库
                System.load(System.getProperty("user.dir") + "/libRaptorQ.so");
            } catch (UnsatisfiedLinkError e2) {
                System.err.println("无法加载RaptorQ库: " + e2.getMessage());
                throw e2;
            }
        }
    }
    
    /**
     * 获取可用块大小列表
     * 
     * @param sizes 用于存储块大小的数组，如果为null则返回可用块大小的总数
     * @param size 数组的大小
     * @return 实际填充的元素数量，如果sizes为null，则返回可用块大小的总数
     */
    public static native int getUsableBlockSizes(int[] sizes, int size);
    
    /**
     * RaptorQ编码函数
     * 
     * @param symbolSize 符号大小（字节）
     * @param symbols 块大小（符号数量）
     * @param repair 修复符号数量
     * @param inputData 输入数据缓冲区
     * @param inputSize 输入数据大小
     * @param outputData 输出数据缓冲区（调用者负责分配足够空间）
     * @param outputSize 输出缓冲区大小
     * @param actualOutputSize 实际输出数据大小（返回值）
     * @return 1表示成功，0表示失败
     */
    public static native int encode(int symbolSize, int symbols, int repair,
                                   byte[] inputData, int inputSize,
                                   byte[] outputData, int outputSize,
                                   int[] actualOutputSize);
    
    /**
     * RaptorQ解码函数
     * 
     * @param bytes 原始数据大小（字节）
     * @param symbols 块大小（符号数量）
     * @param symbolSize 符号大小（字节）
     * @param inputData 输入编码数据缓冲区
     * @param inputSize 输入数据大小
     * @param outputData 输出解码数据缓冲区（调用者负责分配足够空间）
     * @param outputSize 输出缓冲区大小
     * @param actualOutputSize 实际输出数据大小（返回值）
     * @return 1表示成功，0表示失败
     */
    public static native int decode(int bytes, int symbols, int symbolSize,
                                   byte[] inputData, int inputSize,
                                   byte[] outputData, int outputSize,
                                   int[] actualOutputSize);
    
    /**
     * 获取可用块大小的便捷方法
     * 
     * @return 可用块大小数组
     */
    public static int[] getAvailableBlockSizes() {
        // 首先获取可用块大小的数量
        int count = getUsableBlockSizes(null, 0);
        if (count <= 0) {
            return new int[0];
        }
        
        // 分配数组并获取实际的块大小
        int[] sizes = new int[count];
        int actualCount = getUsableBlockSizes(sizes, count);
        
        // 如果实际数量小于预期，创建正确大小的数组
        if (actualCount < count) {
            int[] result = new int[actualCount];
            System.arraycopy(sizes, 0, result, 0, actualCount);
            return result;
        }
        
        return sizes;
    }
    
    /**
     * 编码数据的便捷方法
     * 
     * @param symbolSize 符号大小
     * @param symbols 符号数量
     * @param repair 修复符号数量
     * @param data 要编码的数据
     * @return 编码后的数据，如果编码失败返回null
     */
    public static byte[] encodeData(int symbolSize, int symbols, int repair, byte[] data) {
        if (data == null || data.length == 0) {
            return null;
        }
        
        // 估算输出缓冲区大小（原始数据 + 修复数据）
        int estimatedOutputSize = data.length + (int)(repair * symbolSize);
        byte[] outputBuffer = new byte[estimatedOutputSize];
        int[] actualSize = new int[1];
        
        int result = encode(symbolSize, symbols, repair, data, data.length,
                          outputBuffer, estimatedOutputSize, actualSize);
        
        if (result == 1 && actualSize[0] > 0) {
            // 创建正确大小的输出数组
            byte[] encodedData = new byte[actualSize[0]];
            System.arraycopy(outputBuffer, 0, encodedData, 0, actualSize[0]);
            return encodedData;
        }
        
        return null;
    }
    
    /**
     * 解码数据的便捷方法
     * 
     * @param originalSize 原始数据大小
     * @param symbols 符号数量
     * @param symbolSize 符号大小
     * @param encodedData 编码后的数据
     * @return 解码后的原始数据，如果解码失败返回null
     */
    public static byte[] decodeData(int originalSize, int symbols, int symbolSize, byte[] encodedData) {
        if (encodedData == null || encodedData.length == 0) {
            return null;
        }
        
        byte[] outputBuffer = new byte[originalSize];
        int[] actualSize = new int[1];
        
        int result = decode(originalSize, symbols, symbolSize, encodedData, encodedData.length,
                          outputBuffer, originalSize, actualSize);
        
        if (result == 1 && actualSize[0] > 0) {
            // 创建正确大小的输出数组
            byte[] decodedData = new byte[actualSize[0]];
            System.arraycopy(outputBuffer, 0, decodedData, 0, actualSize[0]);
            return decodedData;
        }
        
        return null;
    }
}
package com.hailiao.util;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

/**
 * RaptorQ库使用示例
 * 演示如何使用KTRaptorQ类进行数据的编码和解码
 */
public class RaptorQExample {
    
    public static void main(String[] args) {
        System.out.println("=== RaptorQ编解码库示例 ===");
        
        try {
            // 1. 获取可用的块大小
            demonstrateBlockSizes();
            
            // 2. 演示基本的编码和解码
            demonstrateBasicEncoding();
            
            // 3. 演示便捷方法的使用
            demonstrateConvenienceMethods();
            
        } catch (Exception e) {
            System.err.println("示例执行失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * 演示获取可用块大小
     */
    private static void demonstrateBlockSizes() {
        System.out.println("\n--- 获取可用块大小 ---");
        
        // 使用便捷方法获取所有可用块大小
        int[] blockSizes = KTRaptorQ.getAvailableBlockSizes();
        System.out.println("可用块大小数量: " + blockSizes.length);
        System.out.println("可用块大小: " + Arrays.toString(blockSizes));
        
        // 也可以手动调用原始方法
        int totalCount = KTRaptorQ.getUsableBlockSizes(null, 0);
        System.out.println("通过原始方法获取的总数量: " + totalCount);
    }
    
    /**
     * 演示基本的编码和解码操作
     */
    private static void demonstrateBasicEncoding() {
        System.out.println("\n--- 基本编码解码示例 ---");
        
        // 准备测试数据
        String testMessage = "Hello, RaptorQ! 这是一个测试消息，用于演示RaptorQ前向纠错编码的功能。";
        byte[] originalData = testMessage.getBytes(StandardCharsets.UTF_8);
        System.out.println("原始数据: " + testMessage);
        System.out.println("原始数据长度: " + originalData.length + " 字节");
        
        // 编码参数
        int symbolSize = 64;  // 符号大小64字节
        int symbols = (originalData.length + symbolSize - 1) / symbolSize; // 计算需要的符号数
        int repair = 10;  // 10个修复符号
        
        System.out.println("编码参数:");
        System.out.println("  符号大小: " + symbolSize + " 字节");
        System.out.println("  符号数量: " + symbols);
        System.out.println("  修复符号: " + repair);
        
        // 估算输出缓冲区大小
        int estimatedOutputSize = (originalData.length + 8) + (int)(repair * symbolSize);
        byte[] encodedBuffer = new byte[estimatedOutputSize];
        int[] actualEncodedSize = new int[1];
        
        // 执行编码
        int encodeResult = KTRaptorQ.encode(
            symbolSize, symbols, repair,
            originalData, originalData.length,
            encodedBuffer, estimatedOutputSize,
            actualEncodedSize
        );
        
        if (encodeResult == 1) {
            System.out.println("编码成功!");
            System.out.println("编码后数据大小: " + actualEncodedSize[0] + " 字节");
            System.out.println("冗余率: " + String.format("%.2f%%", 
                (double)(actualEncodedSize[0] - originalData.length) / originalData.length * 100));
            
            // 创建实际大小的编码数据数组
            byte[] encodedData = new byte[actualEncodedSize[0]];
            System.arraycopy(encodedBuffer, 0, encodedData, 0, actualEncodedSize[0]);
            
            // 执行解码
            byte[] decodedBuffer = new byte[originalData.length];
            int[] actualDecodedSize = new int[1];
            
            int decodeResult = KTRaptorQ.decode(
                originalData.length, symbols, symbolSize,
                encodedData, encodedData.length,
                decodedBuffer, originalData.length,
                actualDecodedSize
            );
            
            if (decodeResult == 1) {
                System.out.println("解码成功!");
                System.out.println("解码后数据大小: " + actualDecodedSize[0] + " 字节");
                
                // 验证数据完整性
                byte[] decodedData = new byte[actualDecodedSize[0]];
                System.arraycopy(decodedBuffer, 0, decodedData, 0, actualDecodedSize[0]);
                
                String decodedMessage = new String(decodedData, StandardCharsets.UTF_8);
                System.out.println("解码后数据: " + decodedMessage);
                
                boolean isIdentical = Arrays.equals(originalData, decodedData);
                System.out.println("数据完整性验证: " + (isIdentical ? "通过" : "失败"));
            } else {
                System.err.println("解码失败!");
            }
        } else {
            System.err.println("编码失败!");
        }
    }
    
    /**
     * 演示便捷方法的使用
     */
    private static void demonstrateConvenienceMethods() {
        System.out.println("\n--- 便捷方法示例 ---");
        
        // 准备测试数据
        String testMessage = "使用便捷方法进行RaptorQ编解码测试。";
        byte[] originalData = testMessage.getBytes(StandardCharsets.UTF_8);
        System.out.println("原始数据: " + testMessage);
        
        // 编码参数
        int symbolSize = 32;
        int symbols = (originalData.length + symbolSize - 1) / symbolSize;
        int repair = 5;
        
        // 使用便捷方法进行编码
        byte[] encodedData = KTRaptorQ.encodeData(symbolSize, symbols, repair, originalData);
        
        if (encodedData != null) {
            System.out.println("便捷编码成功! 编码后大小: " + encodedData.length + " 字节");
            
            // 使用便捷方法进行解码
            byte[] decodedData = KTRaptorQ.decodeData(originalData.length, symbols, symbolSize, encodedData);
            
            if (decodedData != null) {
                String decodedMessage = new String(decodedData, StandardCharsets.UTF_8);
                System.out.println("便捷解码成功! 解码后数据: " + decodedMessage);
                
                boolean isIdentical = Arrays.equals(originalData, decodedData);
                System.out.println("数据完整性验证: " + (isIdentical ? "通过" : "失败"));
            } else {
                System.err.println("便捷解码失败!");
            }
        } else {
            System.err.println("便捷编码失败!");
        }
    }
}
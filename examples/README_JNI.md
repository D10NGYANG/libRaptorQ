# RaptorQ JNI 使用指南

本文档说明如何编译和使用 RaptorQ 库的 Java JNI 接口。

## 文件结构

```
libRaptorQ/
├── src/
│   ├── jni/
│   │   ├── RaptorQ_Jni.cpp          # JNI 实现文件
│   │   └── include/
│   │       ├── jni.h                # JNI 头文件
│   │       └── jni_md.h             # JNI 平台相关头文件
│   └── api/
│       └── RaptorQ_Api.h            # C API 头文件
├── examples/
│   ├── KTRaptorQ.java               # Java 接口类
│   ├── RaptorQExample.java          # 使用示例
│   └── README_JNI.md                # 本文档
└── CMakeLists.txt                   # 已更新包含 JNI 源文件
```

## 编译步骤

### 1. 编译 C++ 动态库

```bash
# 在项目根目录下
mkdir build
cd build

# 配置 CMake（确保启用动态库构建）
cmake .. -DDYNAMIC_LIB=ON -DSTATIC_LIB=OFF

# 编译
make -j$(nproc)

# 编译完成后，动态库位于 build/lib/ 目录下
# macOS: libRaptorQ.dylib
# Linux: libRaptorQ.so
# Windows: RaptorQ.dll
```

### 2. 编译 Java 代码

```bash
# 在 examples 目录下
cd examples

# 编译 Java 文件
javac -d . KTRaptorQ.java RaptorQExample.java
```

### 3. 生成 JNI 头文件（可选，用于验证）

```bash
# 生成 JNI 头文件
javah -jni -classpath . com.hailiao.util.KTRaptorQ

# 这会生成 com_hailiao_util_KTRaptorQ.h 文件
# 可以用来验证 JNI 函数签名是否正确
```

## 运行示例

### 方法 1：使用系统库路径

```bash
# 将动态库复制到系统库路径或设置 LD_LIBRARY_PATH
export LD_LIBRARY_PATH=../build/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=../build/lib:$DYLD_LIBRARY_PATH  # macOS

# 运行示例
java com.hailiao.util.RaptorQExample
```

### 方法 2：指定库路径

```bash
# 使用 java.library.path 系统属性
java -Djava.library.path=../build/lib com.hailiao.util.RaptorQExample
```

### 方法 3：将库文件放在当前目录

```bash
# 复制库文件到当前目录
cp ../build/lib/libRaptorQ.* .  # Linux/macOS
# copy ..\build\lib\RaptorQ.dll .  # Windows

# 运行示例
java com.hailiao.util.RaptorQExample
```

## API 说明

### KTRaptorQ 类

#### 原生方法

- `getUsableBlockSizes(short[] sizes, int size)` - 获取可用块大小列表
- `encode(...)` - RaptorQ 编码
- `decode(...)` - RaptorQ 解码

#### 便捷方法

- `getAvailableBlockSizes()` - 获取所有可用块大小
- `encodeData(...)` - 简化的编码方法
- `decodeData(...)` - 简化的解码方法

### 使用示例

```java
// 获取可用块大小
short[] blockSizes = KTRaptorQ.getAvailableBlockSizes();

// 编码数据
byte[] originalData = "Hello, World!".getBytes();
byte[] encodedData = KTRaptorQ.encodeData(64, (short)1, 5, originalData);

// 解码数据
byte[] decodedData = KTRaptorQ.decodeData(
    originalData.length, (short)1, 64, encodedData);
```

## 故障排除

### 常见错误

1. **UnsatisfiedLinkError: no RaptorQ in java.library.path**
   - 确保动态库在系统库路径中或使用 `-Djava.library.path` 指定路径
   - 检查库文件名是否正确（libRaptorQ.so/dylib 或 RaptorQ.dll）

2. **UnsatisfiedLinkError: ... wrong ELF class**
   - JVM 和动态库的架构不匹配（32位 vs 64位）
   - 确保使用相同架构编译库和运行 Java

3. **编译错误**
   - 确保 JNI 头文件路径正确
   - 检查 CMakeLists.txt 是否正确包含了 JNI 源文件

### 调试技巧

1. 使用 `ldd`（Linux）或 `otool -L`（macOS）检查库依赖：
   ```bash
   ldd libRaptorQ.so  # Linux
   otool -L libRaptorQ.dylib  # macOS
   ```

2. 使用 `nm` 检查库中的符号：
   ```bash
   nm -D libRaptorQ.so | grep Java  # 查看 JNI 函数
   ```

3. 启用 JNI 调试：
   ```bash
   java -Xcheck:jni com.hailiao.util.RaptorQExample
   ```

## 性能优化建议

1. **批量处理**：对于大量小数据，考虑批量编码以减少 JNI 调用开销
2. **内存管理**：避免频繁的大数组分配，考虑重用缓冲区
3. **参数调优**：根据数据特性调整符号大小和修复符号数量

## 许可证

本 JNI 接口遵循与 libRaptorQ 相同的许可证条款（LGPL v3）。
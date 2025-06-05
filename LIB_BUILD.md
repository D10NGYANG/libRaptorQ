# 项目打包

## macOS
```shell
rm -rf build && mkdir build && cd build/ && cmake -DCMAKE_BUILD_TYPE=Release -DUSE_LZ4=BUNDLED -DSTATIC_LIB=ON -DDYNAMIC_LIB=ON .. && cmake --build . --config Release
```

## Android 

```shell
rm -rf build && mkdir build && cd build/ && cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DPROFILING=OFF -DCMAKE_TOOLCHAIN_FILE=/Users/d10ng/Library/android/sdk/ndk/25.2.9519653/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-23 .. && cmake --build .
```

## iOS

```shell
rm -rf build && mkdir build && cd build/ && cmake -G Xcode -DCMAKE_BUILD_TYPE=Release -DPROFILING=OFF -DLTO=OFF -DCLI=OFF -DDYNAMIC_LIB=OFF -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DPLATFORM=OS64 .. && cmake --build . --config Release -- CODE_SIGNING_ALLOWED=NO
```
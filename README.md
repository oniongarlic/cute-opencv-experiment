Using OpenCV from Qt

Tested with Desktop and Android.

Using OpenCV from git 

Build OpenCV for Android with:

~/Android/Sdk/cmake/3.6.4111459/bin/cmake .. 
 -DANDROID_ABI="armeabi-v7a with NEON" 
 -DANDROID_NATIVE_API_LEVEL=23 
 -DANDROID_TOOLCHAIN=gcc 
 -DCMAKE_TOOLCHAIN_FILE=/home/milang/Android/Sdk/ndk-bundle/build/cmake/android.toolchain.cmake 
 -DANDROID_NDK=/home/milang/Android/Sdk/ndk-bundle 
 -DCMAKE_BUILD_TYPE=Release 
 -DANDROID_SDK=/home/milang/Android/Sdk
 -DBUILD_SHARED_LIBS=ON
 -DBUILD_ANDROID_PROJECTS=OFF
 -DANDROID_STL=gnustl_static
 -DWITH_OPENCL=ON
 -DENABLE_NEON=ON
 -DANDROID_ARM_NEON=ON

Copy libraries into 3rdparty/opencv-armv7/



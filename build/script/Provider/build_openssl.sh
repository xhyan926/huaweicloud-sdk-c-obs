#!/bin/bash

open_src_path=`pwd`
# 修改为使用 Tongsuo
echo_openssl_version=`echo ${tongsuo_version}NULL`
if [ ${echo_openssl_version} = "NULL" ]; then
  tongsuo_version=tongsuo-8.3.0
fi
tongsuo_dir=./../../../third_party_groupware/eSDK_Storage_Plugins/${tongsuo_version}
tongsuo_lib=`pwd`/build/${tongsuo_version}/lib
static_tongsuo_lib=`pwd`/build/${tongsuo_version}/static_package/lib
tongsuo_include=`pwd`/build/${tongsuo_version}/include/openssl

cd $tongsuo_dir
chmod 777 Configure
chmod 777 util/point.sh
chmod 777 util/pod2mantest

if [ $# = 0 ]; then
    if [ $BUILD_FOR_ARM = "true" ];then
        CFLAGS="-Wall -O3 -fstack-protector-all -Wl,-z,relro,-z,now" ./Configure threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/ linux-aarch64
    elif [ $BUILD_FOR_NDK_AARCH64 = "true" ];then
        export ANDROID_NDK_HOME=/tmp/ndk-aarch64
        CFLAGS="-Wall -O3 -fstack-protector-all" LDFLAGS="-Wl,-z,relro,-z,now" ./Configure threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/ android-arm64
    elif [ $BUILD_FOR_MACOS = "true" ];then
        CFLAGS="-Wall -O3 -fstack-protector-all" ./config threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/
    else
        # Tongsuo 支持 SM 算法
        CFLAGS="-Wall -O3 -fstack-protector-all -Wl,-z,relro,-z,now" ./Configure threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/ linux-x86_64 -DOPENSSL_NO_ASM
    fi
elif [ $1 = "BUILD_FOR_ARM" ]; then
    CFLAGS="-Wall -O3 -fstack-protector-all -Wl,-z,relro,-z,now" ./Configure threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/ linux-aarch64
elif [ $1 = "BUILD_FOR_NDK_AARCH64" ]; then
    export ANDROID_NDK_HOME=/tmp/ndk-aarch64
    CFLAGS="-Wall -O3 -fstack-protector-all" LDFLAGS="-Wl,-z,relro,-z,now" ./Configure threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/ android-arm64
elif [ $1 = "BUILD_FOR_MACOS" ]; then
    CFLAGS="-Wall -O3 -fstack-protector-all" ./config threads shared --prefix=/usr/local/openssl --openssldir=/usr/local/ssl/
fi

make clean
make
make install

cd $open_src_path
mkdir -p $tongsuo_lib
mkdir -p $tongsuo_include
mkdir -p $static_tongsuo_lib
if [ $1 = "BUILD_FOR_MACOS" ]; then
cp -a ${open_src_path}/${tongsuo_dir}/*.dylib  $tongsuo_lib
else
cp ${open_src_path}/${tongsuo_dir}/libcrypto.so*  $tongsuo_lib
cp ${open_src_path}/${tongsuo_dir}/libssl.so*  $tongsuo_lib
fi
cp ${open_src_path}/${tongsuo_dir}/include/openssl/*.h $tongsuo_include
cp ${open_src_path}/${tongsuo_dir}/*.a  $static_tongsuo_lib

cd $open_src_path

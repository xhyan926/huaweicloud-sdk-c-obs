# 临时授权URL功能编译指南

## 前提条件

1. 已安装华为云OBS C SDK
2. 已安装必要的依赖库：
   - libcurl
   - libpcre
   - libxml2
   - OpenSSL

## 编译步骤

### 1. 确认新文件已添加

确保以下文件存在：
- `inc/temp_url.h` - 头文件
- `src/temp_url.c` - 实现文件
- `test/test_temp_url.c` - 测试文件

### 2. 检查GNUmakefile已更新

确认`GNUmakefile`中`LIBOBS_SOURCES`已包含`temp_url.c`：

```makefile
LIBOBS_SOURCES := bucket.c  object.c request.c request.c request_context.c \
                 response_headers_handler.c simplexml.c util.c log.c \
				 request_util.c general.c error_parser.c temp_url.c
```

### 3. 清理并重新编译

```bash
cd /path/to/eSDK_OBS_API/eSDK_OBS_API_C++

# 清理旧的构建文件
make clean

# 重新编译SDK
make
```

如果编译成功，应该在`build/lib/`目录生成：
- `libeSDKOBS.so` (Linux) 或
- `libeSDKOBS.dylib` (macOS)

### 4. 编译测试程序

```bash
# 编译测试程序
gcc -o test_temp_url test/test_temp_url.c \
    -I./inc -I./include -I./src \
    -L./build/lib -leSDKOBS \
    -lcurl -lpcre -lxml2 -lssl -lcrypto -lz
```

### 5. 运行测试

```bash
# 设置库路径（如果需要）
export LD_LIBRARY_PATH=./build/lib:$LD_LIBRARY_PATH

# 运行测试
./test_temp_url
```

## 编译Demo程序

### 1. 修改配置

编辑`source/eSDK_OBS_API/eSDK_OBS_API_C++_Demo/demo_temp_url.c`，修改以下宏定义为您的实际配置：

```c
#define ACCESS_KEY            "YOUR_ACCESS_KEY"
#define SECRET_ACCESS_KEY     "YOUR_SECRET_ACCESS_KEY"
#define BUCKET_NAME          "your-bucket-name"
#define HOST_NAME            "obs.cn-north-4.myhuaweicloud.com"
#define OBJECT_KEY            "example-object.txt"
```

### 2. 编译Demo

```bash
gcc -o demo_temp_url source/eSDK_OBS_API/eSDK_OBS_API_C++_Demo/demo_temp_url.c \
    -I./inc -I./include -I./src \
    -L./build/lib -leSDKOBS \
    -lcurl -lpcre -lxml2 -lssl -lcrypto -lz
```

### 3. 运行Demo

```bash
export LD_LIBRARY_PATH=./build/lib:$LD_LIBRARY_PATH
./demo_temp_url
```

## Windows编译说明

### 使用Visual Studio

1. 将以下文件添加到项目：
   - `temp_url.h` 到头文件
   - `temp_url.c` 到源文件

2. 在项目配置中启用安全函数：
   ```
   /D _CRT_SECURE_CPP_WARNINGS
   /D _CRT_SECURE_NO_WARNINGS
   ```

3. 链接必要的库：
   - libeSDKOBS.lib
   - libcurl.lib
   - libpcre.lib
   - libxml2.lib
   - libssl.lib
   - libcrypto.lib

## 验证步骤

### 1. 编译验证

确保编译过程中无错误和警告：

```bash
make 2>&1 | grep -i error
# 应该没有输出
```

### 2. 功能验证

运行测试程序验证所有功能：

- 生成GET临时URL
- 生成PUT临时URL
- 生成DELETE临时URL
- 生成HEAD临时URL
- 带版本ID的URL
- 带自定义参数的URL
- 参数验证

### 3. 集成验证

确保新接口已正确导出：

```bash
# Linux
nm -D build/lib/libeSDKOBS.so | grep temp_url

# macOS
nm -g build/lib/libeSDKOBS.dylib | grep temp_url
```

应该看到以下符号：
- `create_get_temp_url`
- `create_put_temp_url`
- `create_delete_temp_url`
- `create_head_temp_url`
- `create_presigned_url`
- `init_temp_url_params`

## 故障排查

### 编译错误：找不到头文件

**错误信息**：
```
temp_url.h: No such file or directory
```

**解决方案**：
确保编译选项包含正确的头文件路径：
```bash
-I./inc -I./include -I./src
```

### 链接错误：未定义的引用

**错误信息**：
```
undefined reference to `HMAC_SHA1'
```

**解决方案**：
确保链接OpenSSL库：
```bash
-lssl -lcrypto
```

### 运行时错误：找不到库

**错误信息**：
```
error while loading shared libraries: libeSDKOBS.so
```

**解决方案**：
设置库路径或安装到系统目录：
```bash
export LD_LIBRARY_PATH=./build/lib:$LD_LIBRARY_PATH
# 或
sudo cp build/lib/libeSDKOBS.so /usr/local/lib/
```

### 测试失败：签名验证失败

**错误信息**：
```
Status: OBS_STATUS_SignatureDoesNotMatch
```

**解决方案**：
1. 检查访问密钥和秘密密钥是否正确
2. 检查系统时间是否准确
3. 检查SDK与OBS服务的时间同步

## 性能测试

### 批量生成URL测试

```c
#include <time.h>

#define NUM_URLS 1000

void test_batch_generate() {
    obs_options options;
    init_obs_options(&options);
    // 设置options...

    clock_t start = clock();

    for (int i = 0; i < NUM_URLS; i++) {
        char url[4096];
        create_get_temp_url(&options, "test.txt", 3600, url, sizeof(url));
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("生成%d个URL耗时: %.3f秒\n", NUM_URLS, elapsed);
    printf("平均每个URL: %.3f毫秒\n", (elapsed * 1000) / NUM_URLS);
}
```

## 发布清单

在发布新版本前，确保完成以下检查：

- [ ] 所有代码通过编译
- [ ] 所有单元测试通过
- [ ] Demo程序可正常运行
- [ ] 文档已更新
- [ ] API变更已记录
- [ ] 版本号已更新
- [ ] Release Notes已更新

## 联系方式

如有问题，请联系：
- 华为云技术支持
- OBS SDK开发团队

# 临时授权URL功能实现文档

## 功能概述

临时授权URL（Pre-signed URL）功能允许在不需要提供访问密钥的情况下临时访问华为云OBS资源。这对于以下场景特别有用：

- 临时文件分享给第三方
- 客户端直接上传/下载文件，无需服务器中转
- 第三方系统集成
- 浏览器直接访问私有对象

## 实现文件

### 头文件
- `inc/temp_url.h` - 临时授权URL功能接口定义

### 实现文件
- `src/temp_url.c` - 临时授权URL功能实现

### 测试文件
- `test/test_temp_url.c` - 单元测试

### Demo文件
- `source/eSDK_OBS_API/eSDK_OBS_API_C++_Demo/demo_temp_url.c` - 使用示例

## API接口

### 1. 初始化参数结构

```c
void init_temp_url_params(obs_temp_url_params *params);
```

### 2. 生成通用临时授权URL

```c
obs_status create_presigned_url(
    const obs_options *options,
    const obs_temp_url_params *params,
    obs_temp_url_result *result
);
```

**参数说明：**
- `options`: SDK配置选项，包含访问密钥等信息
- `params`: 临时URL参数
  - `key`: 对象键名（必填）
  - `version_id`: 版本ID（可选）
  - `expires`: 过期时间（秒，必填）
  - `http_method`: HTTP方法（必填）
  - `query_params`: 查询参数（可选）
  - `query_params_count`: 查询参数数量
- `result`: 输出结果
  - `url`: 生成的临时URL
  - `expires_timestamp`: 过期时间戳
  - `status`: 生成状态
  - `error_message`: 错误消息（如果失败）

### 3. 生成GET临时授权URL

```c
obs_status create_get_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);
```

用于下载对象的临时URL。

### 4. 生成PUT临时授权URL

```c
obs_status create_put_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    const char *content_type,
    char *url_out,
    int url_out_len
);
```

用于上传对象的临时URL。

### 5. 生成DELETE临时授权URL

```c
obs_status create_delete_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);
```

用于删除对象的临时URL。

### 6. 生成HEAD临时授权URL

```c
obs_status create_head_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);
```

用于获取对象元数据的临时URL。

## 使用示例

### 基本示例：生成下载URL

```c
#include "eSDKOBS.h"

// 初始化SDK
obs_initialize(OBS_INIT_ALL);

// 配置选项
obs_options options;
init_obs_options(&options);
options.bucket_options.access_key = "YOUR_ACCESS_KEY";
options.bucket_options.secret_access_key = "YOUR_SECRET_ACCESS_KEY";
options.bucket_options.bucket_name = "your-bucket-name";
options.bucket_options.host_name = "obs.cn-north-4.myhuaweicloud.com";
options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

// 生成临时下载URL（7天有效）
char temp_url[4096];
uint64_t expires = 604800; // 7天 = 7 * 24 * 3600 秒
obs_status status = create_get_temp_url(&options, "example-object.txt",
                                              expires, temp_url, sizeof(temp_url));

if (status == OBS_STATUS_OK) {
    printf("临时URL: %s\n", temp_url);
    // 使用curl下载
    // curl temp_url
} else {
    printf("生成失败: %s\n", obs_get_status_name(status));
}

// 清理SDK
obs_deinitialize();
```

### 高级示例：带自定义参数的临时URL

```c
// 设置自定义查询参数
obs_name_value query_params[2];
obs_name_value param1 = { "response-content-type", "application/json" };
obs_name_value param2 = { "response-cache-control", "no-cache" };
query_params[0] = param1;
query_params[1] = param2;

// 设置临时URL参数
obs_temp_url_params params;
init_temp_url_params(&params);
params.key = "example-object.json";
params.expires = 3600; // 1小时
params.http_method = OBS_HTTP_METHOD_GET;
params.query_params = query_params;
params.query_params_count = 2;

// 生成临时URL
obs_temp_url_result result;
obs_status status = create_presigned_url(&options, &params, &result);

if (status == OBS_STATUS_OK) {
    printf("临时URL: %s\n", result.url);
    printf("过期时间戳: %lld\n", result.expires_timestamp);
}
```

## 安全建议

1. **合理设置过期时间**
   - 仅设置实际需要的有效期
   - 对于长期分享，建议7天（604800秒）
   - 对于即时操作，建议5-15分钟

2. **不要记录URL**
   - 避免在日志中打印临时URL
   - 如果必须记录，应脱敏处理

3. **使用HTTPS**
   - 始终使用HTTPS协议传输敏感数据
   - 仅在可信网络中使用HTTP

4. **限制权限**
   - 根据需求选择适当的HTTP方法（GET/PUT/DELETE）
   - 使用最小权限原则

5. **版本控制**
   - 对于需要访问特定版本的场景，使用version_id参数

## 编译说明

### 1. 构建库

```bash
cd /path/to/eSDK_OBS_API/eSDK_OBS_API_C++
make clean
make
```

### 2. 运行测试

```bash
cd /path/to/eSDK_OBS_API/eSDK_OBS_API_C++
gcc -o test_temp_url test/test_temp_url.c \
    -I./inc -I./include -I./src \
    -L./build/lib -leSDKOBS \
    -lcurl -lpcre -lxml2 -lssl -lcrypto -lz

./test_temp_url
```

### 3. 运行Demo

修改`demo_temp_url.c`中的配置信息后：

```bash
gcc -o demo_temp_url source/eSDK_OBS_API/eSDK_OBS_API_C++_Demo/demo_temp_url.c \
    -I./inc -I./include -I./src \
    -L./build/lib -leSDKOBS \
    -lcurl -lpcre -lxml2 -lssl -lcrypto -lz

./demo_temp_url
```

## 已知限制

1. **过期时间限制**
   - 最长7天（604800秒）
   - 由华为云OBS服务端限制

2. **URL长度**
   - 最大URL长度4096字节
   - 包含签名和参数

3. **签名算法**
   - 当前使用AWS Signature V2
   - 支持OBS和S3兼容API

## 故障排查

### 错误：OBS_STATUS_NoToken
**原因**: 未提供访问密钥或秘密密钥

**解决**: 确保正确设置`access_key`和`secret_access_key`

### 错误：OBS_STATUS_InvalidParameter
**原因**: 参数验证失败

**解决**: 检查必填参数是否正确

### 错误：OBS_STATUS_UriTooLong
**原因**: 生成的URL超过最大长度

**解决**: 减少查询参数或缩短对象键名

## 下一步计划

1. **支持AWS Signature V4**
   - 提供更安全的签名算法
   - 支持区域化签名

2. **POST对象签名**
   - 支持HTML表单上传
   - 生成表单签名

3. **性能优化**
   - 缓存签名计算
   - 批量生成URL

## 版本历史

- v1.0 - 初始版本（2024年）
  - 实现基础临时URL生成功能
  - 支持GET/PUT/DELETE/HEAD方法
  - 支持版本ID和自定义查询参数

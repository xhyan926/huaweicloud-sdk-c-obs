# 桶加密配置功能实现文档

## 功能概述

桶加密配置功能允许用户为桶设置默认的服务器端加密（SSE），确保新上传到桶的对象自动使用指定的加密方式。

### 支持的加密类型

1. **OBS_BUCKET_ENCRYPTION_KMS** - SSE-KMS（Server-Side Encryption with KMS）
   - 使用华为云KMS（Key Management Service）管理密钥
   - 适合企业级安全需求
   - 符合国内合规要求（国密标准）

2. **OBS_BUCKET_ENCRYPTION_AES256** - SSE-AES256（Server-Side Encryption with AES256）
   - 使用AES256算法加密
   - 华为云管理加密密钥
   - 适合大多数使用场景

3. **OBS_BUCKET_ENCRYPTION_SM4** - SSE-SM4（Server-Side Encryption with SM4）
   - 使用国密算法（SM2/SM3/SM4）
   - 适合有国密合规要求的场景

---

## API接口

### 1. 设置桶加密配置

```c
/**
 * 设置桶加密配置
 *
 * @param options SDK配置选项
 * @param config 桶加密配置
 * @param handler 响应处理器
 * @param callback_data 回调数据
 */
void set_bucket_encryption(
    const obs_options *options,
    const obs_bucket_encryption_config *config,
    obs_response_handler *handler,
    void *callback_data
);
```

**参数说明**：
- `options`: SDK配置选项
  - `bucket_options.bucket_name`: 桶名称
  - `bucket_options.access_key`: 访问密钥
  - `bucket_options.secret_access_key`: 秘密钥
  - `bucket_options.host_name`: 终端节点
  - `bucket_options.protocol`: 协议（HTTP/HTTPS）

- `config`: 加密配置
  - `encryption_type`: 加密类型（KMS/AES256/SM4）
  - `kms_key_id`: KMS密钥ID（仅SSE-KMS使用）
  - `sse_algorithm`: SSE算法（仅SSE-C使用，当前未实现）

- `handler`: 响应处理器
  - `properties_callback`: 属性回调（可选）
  - `complete_callback`: 完成回调（必填）

**返回**：异步执行，结果通过回调返回

**使用示例**：
```c
// 设置SSE-KMS加密
obs_bucket_encryption_config config;
memset_s(&config, 0, sizeof(config));
config.encryption_type = OBS_BUCKET_ENCRYPTION_KMS;
config.kms_key_id = "arn:aws:kms:region:cn-north-4:1234567890123:key/test-kms-key-id";

set_bucket_encryption(&options, &config, &response_handler, NULL);
```

---

### 2. 获取桶加密配置

```c
/**
 * 获取桶加密配置
 *
 * @param options SDK配置选项
 * @param handler 桶加密配置处理器
 * @param callback_data 回调数据
 */
void get_bucket_encryption(
    const obs_options *options,
    obs_get_bucket_encryption_handler *handler,
    void *callback_data
);
```

**参数处理器**：
```c
typedef obs_status (obs_get_bucket_encryption_callback)(
    const obs_bucket_encryption_config *config,
    void *callback_data
);

typedef struct obs_get_bucket_encryption_handler {
    obs_response_handler response_handler;
    obs_get_bucket_encryption_callback *get_encryption_callback;
} obs_get_bucket_encryption_handler;
```

**使用示例**：
```c
// 获取加密配置
obs_bucket_encryption_config config_return;
memset_s(&config_return, 0, sizeof(config_return));

obs_get_bucket_encryption_handler handler;
memset_s(&handler, 0, sizeof(handler));
handler.response_handler.properties_callback = &properties_callback;
handler.response_handler.complete_callback = &complete_callback;
handler.get_encryption_callback =
    [](const obs_bucket_encryption_config *config, void *callback_data) {
        if (config != NULL) {
            printf("加密类型: ");
            switch (config->encryption_type) {
                case OBS_BUCKET_ENCRYPTION_KMS:
                    printf("SSE-KMS\n");
                    printf("KMS密钥ID: %s\n", config->kms_key_id);
                    break;
                case OBS_BUCKET_ENCRYPTION_AES256:
                    printf("AES256\n");
                    break;
                case OBS_BUCKET_ENCRYPTION_SM4:
                    printf("SM4（国密）\n");
                    break;
                default:
                    printf("未知\n");
                    break;
            }
        }
    };

get_bucket_encryption(&options, &handler, NULL);
```

---

### 3. 删除桶加密配置

```c
/**
 * 删除桶加密配置
 *
 * @param options SDK配置选项
 * @param handler 响应处理器
 * @param callback_data 回调数据
 */
void delete_bucket_encryption(
    const obs_options *options,
    obs_response_handler *handler,
    void *callback_data
);
```

**使用示例**：
```c
// 删除加密配置
delete_bucket_encryption(&options, &response_handler, NULL);
```

**注意**：
- 删除加密配置后，新上传的对象将不再自动加密
- 已存在的对象不受影响
- 可以重新设置加密配置

---

## 数据结构

### 桶加密配置结构

```c
/**
 * 桶加密类型枚举
 */
typedef enum
{
    OBS_BUCKET_ENCRYPTION_AES256 = 0,     // 使用AES256加密
    OBS_BUCKET_ENCRYPTION_KMS = 1,        // 使用KMS加密
    OBS_BUCKET_ENCRYPTION_SM4 = 2          // 使用SM4加密（国密）
} obs_bucket_encryption_type;

/**
 * 桶加密配置结构
 * 用于设置桶的默认服务器端加密配置
 */
typedef struct obs_bucket_encryption_config
{
    obs_bucket_encryption_type encryption_type;    // 加密类型
    const char *kms_key_id;                     // KMS密钥ID（SSE-KMS使用）
    const char *sse_algorithm;                   // SSE算法（SSE-C使用）
} obs_bucket_encryption_config;

/**
 * 桶加密配置回调
 * 用于获取桶加密配置时的回调
 */
typedef obs_status (obs_get_bucket_encryption_callback)(
    const obs_bucket_encryption_config *config,
    void *callback_data
);

/**
 * 桶加密配置处理器
 */
typedef struct obs_get_bucket_encryption_handler {
    obs_response_handler response_handler;
    obs_get_bucket_encryption_callback *get_encryption_callback;
} obs_get_bucket_encryption_handler;
```

---

## 加密类型详细说明

### SSE-KMS（OBS_BUCKET_ENCRYPTION_KMS）

**特性**：
- 使用华为云KMS管理密钥
- 无需管理密钥生命周期
- 自动密钥轮换
- 符合国密标准（

**适用场景**：
- 企业级数据加密
- GDPR等数据保护法规
- 金融、医疗行业数据存储
- 需要密钥审计和管理的场景

**配置要求**：
- 配置有效的KMS密钥ID
- 密钥必须在同一区域创建
- 密钥必须具有KMS加密权限

**示例配置**：
```c
obs_bucket_encryption_config config;
config.encryption_type = OBS_BUCKET_ENCRYPTION_KMS;
config.kms_key_id = "arn:aws:kms:region:cn-north-4:1234567890123:key/my-kms-key";
```

### SSE-AES256（OBS_BUCKET_ENCRYPTION_AES256）

**特性**：
- 使用AES256算法
- 华为云管理加密密钥
- 无需额外配置

**适用场景**：
- 一般数据加密需求
- 云应用数据存储
- 成本敏感但不需要KMS的场景

**示例配置**：
```c
obs_bucket_encryption_config config;
config.encryption_type = OBS_BUCKET_ENCRYPTION_AES256;
```

### SSE-SM4（OBS_BUCKET_ENCRYPTION_SM4）

**特性**：
- 使用国密算法（SM2/SM3/SM4）
- 符合国内密码法规
- 使用Tongsuo等国密SSL库

**适用场景**：
- 有国密合规要求
- 政府/金融等高安全等级应用
- 需要使用国密算法的场景

**示例配置**：
```c
obs_bucket_encryption_config config;
config.encryption_type = OBS_BUCKET_ENCRYPTION_SM4;
```

---

## XML格式

### 设置加密配置的XML格式

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ServerSideEncryptionConfiguration xmlns="http://obs.myhuaweicloud.com/doc/2015-06-30/">
    <Rule>
        <ApplyServerSideEncryptionByDefault>
            <SSE-KMS>
                <KMSKeyId>arn:aws:kms:region:cn-north-4:1234567890123:key/test-key-id</KMSKeyId>
            </SSE-KMS>
        </ApplyServerSideEncryptionByDefault>
    </Rule>
</ServerSideEncryptionConfiguration>
```

### 删除加密配置的XML格式

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ServerSideEncryptionConfiguration xmlns="http://obs.myhuaweicloud.com/doc/2015-06-30/">
</ServerSideEncryptionEncryptionConfiguration>
```

---

## 使用场景

### 场景1：企业数据加密（使用SSE-KMS）

```c
#include "eSDKOBS.h"

// 初始化SDK
obs_initialize(OBS_INIT_ALL);

// 配置选项
obs_options options;
init_obs_options(&options);
options.bucket_options.access_key = "YOUR_ACCESS_KEY";
options.bucket_options.secret_access_key = "YOUR_SECRET_ACCESS_KEY";
options.bucket_options.bucket_name = "enterprise-data-bucket";
options.bucket_options.host_name = "obs.cn-north-4.myhuaweicloud.com";
options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

// 设置SSE-KMS加密
obs_bucket_encryption_config config;
memset_s(&config, 0, sizeof(config));
config.encryption_type = OBS_BUCKET_ENCRYPTION_KMS;
config.kms_key_id = "arn:aws:kms:region:cn-north-4:1234567890123:key/enterprise-kms-key-id";

// 设置回调
obs_response_handler response_handler;
memset_s(&response_handler, 0, sizeof(response_handler));
response_handler.complete_callback =
    [](obs_status status, const obs_error_details *error_details, void *callback_data) {
        if (status == OBS_STATUS_OK) {
            printf("桶加密配置设置成功\n");
        } else {
            printf("桶加密配置设置失败: %s\n", obs_get_status_name(status));
        }
    };

// 调用设置接口
set_bucket_encryption(&options, &config, &response_handler, NULL);

// 清理SDK
obs_deinitialize();
```

### 场景2：一般数据加密（使用AES256）

```c
// 设置AES256加密
obs_bucket_encryption_config config;
memset_s(&config, 0, sizeof(config));
config.encryption_type = OBS_BUCKET_ENCRYPTION_AES256;

set_bucket_encryption(&options, &config, &response_handler, NULL);
```

### 场景3：国密合规（使用SM4）

```c
// 设置SM4国密加密
obs_bucket_encryption_config config;
memset_s(&config, 0, sizeof(config));
config.encryption_type = OBS_BUCKET_ENCRYPTION_SM4;

set_bucket_encryption(&options, &config, &response_handler, NULL);
```

### 场景4：查询当前加密配置

```c
// 获取加密配置
obs_bucket_encryption_config config_return;
memset_s(&config_return, 0, sizeof(config_return));

obs_get_bucket_encryption_handler handler;
memset_s(&handler, 0, sizeof(handler));
handler.response_handler.complete_callback =
    [](obs_status status, const obs_error_details *error_details, void *callback_data) {
        if (status == OBS_STATUS_OK) {
            printf("获取加密配置成功\n");
        }
    };

handler.get_encryption_callback =
    [](const obs_bucket_encryption_config *config, void *callback_data) {
        if (config != NULL) {
            printf("当前加密类型: %d\n", config->encryption_type);
            printf("KMS密钥ID: %s\n", config->kms_key_id);
        }
    };

get_bucket_encryption(&options, &handler, &config_return);
```

### 场景5：禁用加密（删除配置）

```c
// 删除加密配置
delete_bucket_encryption(&options, &response_handler, NULL);
```

---

## 错误处理

### 常见错误码

| 错误码 | 说明 |
|--------|------|
| OBS_STATUS_InvalidParameter | 参数无效（如NULL指针）|
| OBS_STATUS_XmlDocumentTooLarge | XML文档过大 |
| OBS_STATUS_ConnectionFailed | 连接失败 |
| OBS_STATUS_Ok | 操作被拒绝（权限不足）|
| OBS_STATUS_InvalidBucketName | 桶名称无效|
| OBS_STATUS_ServiceFailure | 服务端处理失败|

### 错误处理示例

```c
response_handler.complete_callback =
    [](obs_status status, const obs_error_details *error_details, void *callback_data) {
        if (status != OBS_STATUS_OK) {
            printf("操作失败: %s\n", obs_get_status_name(status));

            if (error_details != NULL) {
                if (error_details->message != NULL) {
                    printf("错误信息: %s\n", error_details->message);
                }
                if (error_details->resource != NULL) {
                    printf("错误资源: %s\n", error_details->resource);
                }
            }
        }
    }
};
```

---

## 安全建议

1. **密钥管理**
   - 使用IAM策略最小化KMS密钥权限
   - 定期轮换KMS密钥
   - 启用密钥审计日志

2. **访问控制**
   - 确保只有授权用户可以设置加密配置
   - 使用桶策略限制配置权限

3. **传输安全**
   - 始终使用HTTPS协议
   - 确保在可信网络中传输

4. **数据备份**
   - 在启用加密前备份重要数据
   - 测试加密配置对数据访问的影响

---

## 性能考虑

### 加密性能影响

| 加密类型 | 性能影响 | 磁明 |
|---------|---------|------|
| SSE-KMS | 中等 | 首次请求需要KMS验证 |
| AES256 | 轻微 | 硬件加密开销小 |
| SM4 | 较低 | 国密算法相对较慢 |

### 优化建议

1. 对于大文件上传，考虑使用分片上传
2. 合理设置并发上传数量
3. 监控加密操作的性能指标

---

## 注意事项

1. **桶级别配置**
   - 加密配置是桶级别的，影响所有新上传的对象
   - 已存在的对象不会被自动加密
   - 要加密已有对象，需要复制到新对象

2. **加密不可逆**
   - 一旦对象被加密，无法通过API解密
   - 删除加密配置不影响已加密的对象



3. **KMS密钥要求**
   - KMS密钥必须与桶在同一区域
   - 密钥必须具有正确的IAM权限
   - 建议使用专用的密钥而非主密钥

4. **国密支持**
   - SM4加密需要配置国密SSL库
   - 需要确保使用兼容的Tongsuo版本

---

## 故障排查

### 问题：设置加密配置失败

**可能原因**：
1. KMS密钥ID无效或不存在
2. 密钥权限不足
3. 桶不存在
4. 访问权限不足

**排查步骤**：
```bash
# 1. 验证KMS密钥
aws kms describe-key --key-id your-key-id

# 2. 检查IAM权限
aws iam list-attached-role-policies --role-name your-role

# 3. 验证桶
aws s3api head-bucket your-bucket-name
```

### 问题：获取加密配置返回加密类型为未知

**可能原因**：
1. 桶未设置加密配置
2. XML解析失败

**排查步骤**：
1. 检查桶是否存在
2. 使用华为云控制台验证配置
3. 检查SDK日志获取详细错误信息

---

## 与其他功能的交互

### 与对象级加密的关系

- 桶加密设置**默认**加密方式
- 对象级加密（`server_side_encryption_params`）可以**覆盖**桶的默认设置
- 对象级加密优先级高于桶级加密

### 与访问控制的关系

- 加密不替代ACL
- 需要正确的读取权限才能访问加密对象
- 加密在服务端进行，客户端透明

---

## 版本历史

- v1.0 - 初始版本（2024-02-04）
  - 实现基础桶加密配置功能
  - 支持SSE-KMS、AES256、SM4加密类型
  - 完现设置、获取、删除接口

/*********************************************************************************
* Copyright 2024 Huawei Technologies Co.,Ltd.
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use
* this file except in compliance with the License.  You may obtain a copy of the
* License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed
* under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations under the License.
**********************************************************************************/

/**
 * 华为云OBS SDK - 临时授权URL Demo
 *
 * 本Demo展示如何使用临时授权URL功能，包括：
 * 1. 生成GET临时授权URL（用于下载对象）
 * 2. 生成PUT临时授权URL（用于上传对象）
 * 3. 生成DELETE临时授权URL（用于删除对象）
 * 4. 生成HEAD临时授权URL（用于获取对象元数据）
 * 5. 使用通用接口创建临时URL
 * 6. 带版本ID和自定义参数的临时URL
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eSDKOBS.h"
#include "temp_url.h"

// 配置信息 - 请替换为您的实际配置
#define ACCESS_KEY            "YOUR_ACCESS_KEY"
#define SECRET_ACCESS_KEY     "YOUR_SECRET_ACCESS_KEY"
#define BUCKET_NAME          "your-bucket-name"
#define HOST_NAME            "obs.cn-north-4.myhuaweicloud.com"
#define OBJECT_KEY            "example-object.txt"
#define VERSION_ID           ""  // 可选：如果需要访问特定版本

// 临时URL过期时间（秒），7天 = 7 * 24 * 3600 = 604800秒
#define EXPIRES_TIME_DEFAULT 604800

/**
 * Demo 1: 生成GET临时授权URL（用于下载对象）
 *
 * 场景：需要临时分享文件给他人下载，无需提供访问密钥
 */
void demo_get_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 1: 生成GET临时授权URL（用于下载对象）\n");
    printf("========================================\n");

    // 初始化SDK选项
    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    // 生成GET临时URL
    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_get_temp_url(&options, OBJECT_KEY,
                                                EXPIRES_TIME_DEFAULT,
                                                temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成GET临时授权URL:\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("  过期时间: %llu 秒（约 %.1f 天）\n",
               (unsigned long long)EXPIRES_TIME_DEFAULT,
               EXPIRES_TIME_DEFAULT / 86400.0);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n使用方法:\n");
        printf("  使用浏览器或curl访问上述URL即可下载对象\n");
        printf("  示例: curl \"%s\"\n", temp_url);
    } else {
        printf("生成GET临时授权URL失败: %s\n", obs_get_status_name(status));
        printf("  请检查配置信息是否正确\n");
    }
}

/**
 * Demo 2: 生成PUT临时授权URL（用于上传对象）
 *
 * 场景：允许客户端直接上传文件到OBS，无需服务器中转
 */
void demo_put_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 2: 生成PUT临时授权URL（用于上传对象）\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    // 生成PUT临时URL（指定内容类型）
    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_put_temp_url(&options, OBJECT_KEY,
                                                EXPIRES_TIME_DEFAULT,
                                                "text/plain",
                                                temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成PUT临时授权URL:\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("  内容类型: text/plain\n");
        printf("  过期时间: %llu 秒\n", (unsigned long long)EXPIRES_TIME_DEFAULT);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n使用方法:\n");
        printf("  使用curl上传文件:\n");
        printf("  curl -X PUT -T local-file.txt \"%s\"\n", temp_url);
        printf("  或使用Python requests:\n");
        printf("  import requests\n");
        printf("  with open('local-file.txt', 'rb') as f:\n");
        printf("      response = requests.put('%s', data=f)\n", temp_url);
    } else {
        printf("生成PUT临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 3: 生成DELETE临时授权URL（用于删除对象）
 *
 * 场景：临时授权删除特定对象
 */
void demo_delete_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 3: 生成DELETE临时授权URL（用于删除对象）\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_delete_temp_url(&options, OBJECT_KEY,
                                                   EXPIRES_TIME_DEFAULT,
                                                   temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成DELETE临时授权URL:\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("  过期时间: %llu 秒\n", (unsigned long long)EXPIRES_TIME_DEFAULT);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n使用方法:\n");
        printf("  使用curl删除对象:\n");
        printf("  curl -X DELETE \"%s\"\n", temp_url);
    } else {
        printf("生成DELETE临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 4: 生成HEAD临时授权URL（用于获取对象元数据）
 *
 * 场景：临时授权获取对象信息（不下载内容）
 */
void demo_head_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 4: 生成HEAD临时授权URL（用于获取对象元数据）\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_head_temp_url(&options, OBJECT_KEY,
                                                EXPIRES_TIME_DEFAULT,
                                                temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成HEAD临时授权URL:\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("  过期时间: %llu 秒\n", (unsigned long long)EXPIRES_TIME_DEFAULT);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n使用方法:\n");
        printf("  使用curl获取对象元数据:\n");
        printf("  curl -I \"%s\"\n", temp_url);
    } else {
        printf("生成HEAD临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 5: 使用通用接口创建临时URL（高级用法）
 *
 * 场景：需要更多自定义参数时使用通用接口
 */
void demo_generic_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 5: 使用通用接口创建临时URL（高级用法）\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    // 设置临时URL参数
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = OBJECT_KEY;
    params.expires = EXPIRES_TIME_DEFAULT;
    params.http_method = OBS_HTTP_METHOD_GET;

    // 如果有版本ID，可以设置
    if (strlen(VERSION_ID) > 0) {
        params.version_id = VERSION_ID;
        printf("  使用版本ID: %s\n", VERSION_ID);
    }

    // 生成临时URL
    obs_temp_url_result result;
    obs_status status = create_presigned_url(&options, &params, &result);

    if (status == OBS_STATUS_OK) {
        printf("成功生成临时授权URL:\n");
        printf("  对象键: %s\n", params.key);
        printf("  HTTP方法: GET\n");
        printf("  过期时间: %llu 秒\n", (unsigned long long)params.expires);
        printf("  过期时间戳: %lld\n", result.expires_timestamp);
        printf("\n  临时URL:\n  %s\n", result.url);
    } else {
        printf("生成临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 6: 带自定义查询参数的临时URL
 *
 * 场景：需要设置响应头或其他查询参数
 */
void demo_temp_url_with_params(void)
{
    printf("\n========================================\n");
    printf("Demo 6: 带自定义查询参数的临时URL\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    // 设置自定义查询参数（用于设置响应头）
    obs_name_value query_params[3];
    obs_name_value param1 = { "response-content-type", "application/json" };
    obs_name_value param2 = { "response-cache-control", "no-cache" };
    obs_name_value param3 = { "response-expires", "0" };

    query_params[0] = param1;
    query_params[1] = param2;
    query_params[2] = param3;

    // 设置临时URL参数
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = OBJECT_KEY;
    params.expires = EXPIRES_TIME_DEFAULT;
    params.http_method = OBS_HTTP_METHOD_GET;
    params.query_params = query_params;
    params.query_params_count = 3;

    // 生成临时URL
    obs_temp_url_result result;
    obs_status status = create_presigned_url(&options, &params, &result);

    if (status == OBS_STATUS_OK) {
        printf("成功生成带自定义参数的临时授权URL:\n");
        printf("  对象键: %s\n", params.key);
        printf("  自定义参数:\n");
        printf("    response-content-type: application/json\n");
        printf("    response-cache-control: no-cache\n");
        printf("    response-expires: 0\n");
        printf("\n  临时URL:\n  %s\n", result.url);
    } else {
        printf("生成临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 7: 短期临时URL（用于即时操作）
 *
 * 场景：生成非常短期的临时URL用于即时操作，提高安全性
 */
void demo_short_lived_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 7: 短期临时URL（用于即时操作）\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    // 生成5分钟有效期的临时URL
    uint64_t short_expires = 300; // 5分钟 = 300秒

    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_get_temp_url(&options, OBJECT_KEY,
                                                short_expires,
                                                temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成短期临时授权URL:\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("  过期时间: %llu 秒（5分钟）\n",
               (unsigned long long)short_expires);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n注意:\n");
        printf("  此URL仅在5分钟内有效，适合即时操作\n");
    } else {
        printf("生成短期临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * Demo 8: 使用HTTP协议的临时URL
 *
 * 场景：在非HTTPS环境中使用临时URL
 */
void demo_http_temp_url(void)
{
    printf("\n========================================\n");
    printf("Demo 8: 使用HTTP协议的临时URL\n");
    printf("========================================\n");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = ACCESS_KEY;
    options.bucket_options.secret_access_key = SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = BUCKET_NAME;
    options.bucket_options.host_name = HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTP; // 使用HTTP而非HTTPS

    char temp_url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_get_temp_url(&options, OBJECT_KEY,
                                                EXPIRES_TIME_DEFAULT,
                                                temp_url, sizeof(temp_url));

    if (status == OBS_STATUS_OK) {
        printf("成功生成HTTP协议临时授权URL:\n");
        printf("  协议: HTTP\n");
        printf("  对象键: %s\n", OBJECT_KEY);
        printf("\n  临时URL:\n  %s\n", temp_url);
        printf("\n注意:\n");
        printf("  HTTP协议不加密，仅在可信网络中使用\n");
    } else {
        printf("生成HTTP协议临时授权URL失败: %s\n", obs_get_status_name(status));
    }
}

/**
 * 打印使用说明
 */
void print_usage_info(void)
{
    printf("\n");
    printf("========================================\n");
    printf("临时授权URL使用说明\n");
    printf("========================================\n");
    printf("\n1. 配置信息:\n");
    printf("   - 请修改本文件中的以下宏定义为您的实际配置:\n");
    printf("     * ACCESS_KEY: 华为云访问密钥ID\n");
    printf("     * SECRET_ACCESS_KEY: 华为云秘密访问密钥\n");
    printf("     * BUCKET_NAME: 桶名称\n");
    printf("     * HOST_NAME: 终端节点地址\n");
    printf("     * OBJECT_KEY: 对象键名\n");
    printf("\n2. 过期时间:\n");
    printf("   - 默认7天（604800秒）\n");
    printf("   - 最长7天\n");
    printf("   - 建议根据实际需求设置\n");
    printf("\n3. 安全建议:\n");
    printf("   - 只在需要时生成临时URL\n");
    printf("   - 使用合适的过期时间\n");
    printf("   - 不要在日志中打印临时URL\n");
    printf("   - HTTPS协议更安全\n");
    printf("\n4. 应用场景:\n");
    printf("   - 临时文件分享\n");
    printf("   - 客户端直接上传\n");
    printf("   - 第三方系统集成\n");
    printf("   - 无需服务器中转的操作\n");
    printf("\n");
}

/**
 * 主函数
 */
int main(int argc, char *argv[])
{
    printf("========================================\n");
    printf("华为云OBS SDK - 临时授权URL功能Demo\n");
    printf("========================================\n");

    // 初始化SDK
    obs_status init_status = obs_initialize(OBS_INIT_ALL);
    if (init_status != OBS_STATUS_OK) {
        printf("SDK初始化失败: %s\n", obs_get_status_name(init_status));
        printf("\n请确保已正确安装华为云OBS SDK\n");
        return 1;
    }

    // 运行所有Demo
    demo_get_temp_url();
    demo_put_temp_url();
    demo_delete_temp_url();
    demo_head_temp_url();
    demo_generic_temp_url();
    demo_temp_url_with_params();
    demo_short_lived_temp_url();
    demo_http_temp_url();

    // 清理SDK
    obs_deinitialize();

    // 打印使用说明
    print_usage_info();

    printf("========================================\n");
    printf("Demo执行完成\n");
    printf("========================================\n");

    return 0;
}

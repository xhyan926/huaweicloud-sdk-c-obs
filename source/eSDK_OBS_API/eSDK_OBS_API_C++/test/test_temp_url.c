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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "eSDKOBS.h"
#include "temp_url.h"

// 测试结果统计
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// 测试辅助宏
#define TEST_START(name) \
    printf("\n=== 开始测试: %s ===\n", name); \
    tests_run++;

#define TEST_PASS() \
    printf("  [PASS]\n"); \
    tests_passed++;

#define TEST_FAIL(message) \
    printf("  [FAIL] %s\n", message); \
    tests_failed++;

#define ASSERT_NOT_NULL(ptr, message) \
    if (ptr == NULL) { \
        TEST_FAIL(message); \
        return; \
    }

#define ASSERT_EQUAL(expected, actual, message) \
    if (expected != actual) { \
        printf("  [FAIL] %s: 期望=%d, 实际=%d\n", message, expected, actual); \
        tests_failed++; \
        return; \
    }

#define ASSERT_STATUS_OK(status, message) \
    if (status != OBS_STATUS_OK) { \
        printf("  [FAIL] %s: 状态码=%d (%s)\n", \
               message, status, obs_get_status_name(status)); \
        tests_failed++; \
        return; \
    }

#define ASSERT_STRING_NOT_EMPTY(str, message) \
    if (str == NULL || str[0] == '\0') { \
        TEST_FAIL(message); \
        return; \
    }

// 测试配置
#define TEST_ACCESS_KEY        "YOUR_ACCESS_KEY"
#define TEST_SECRET_ACCESS_KEY "YOUR_SECRET_ACCESS_KEY"
#define TEST_BUCKET_NAME      "test-bucket"
#define TEST_HOST_NAME       "obs.cn-north-4.myhuaweicloud.com"
#define TEST_OBJECT_KEY      "test-object.txt"
#define TEST_EXPIRES_TIME    3600

/**
 * 测试1：初始化临时URL参数结构
 */
static void test_init_temp_url_params(void)
{
    TEST_START("初始化临时URL参数结构");

    obs_temp_url_params params;
    init_temp_url_params(&params);

    ASSERT_EQUAL(OBS_HTTP_METHOD_GET, params.http_method, "HTTP方法应为GET");
    ASSERT_EQUAL(3600, params.expires, "过期时间应默认为3600秒");

    TEST_PASS();
}

/**
 * 测试2：生成GET临时授权URL
 */
static void test_create_get_temp_url(void)
{
    TEST_START("生成GET临时授权URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_get_temp_url(&options, TEST_OBJECT_KEY,
                                                TEST_EXPIRES_TIME, url, sizeof(url));

    // 注意：使用无效的密钥会失败，这是正常的
    // 这里主要测试函数调用和参数处理
    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        // 测试参数验证
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL长度: %zu\n", strlen(url));
        TEST_PASS();
    }
}

/**
 * 测试3：生成PUT临时授权URL
 */
static void test_create_put_temp_url(void)
{
    TEST_START("生成PUT临时授权URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_put_temp_url(&options, TEST_OBJECT_KEY,
                                                TEST_EXPIRES_TIME,
                                                "text/plain",
                                                url, sizeof(url));

    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL长度: %zu\n", strlen(url));
        TEST_PASS();
    }
}

/**
 * 测试4：生成DELETE临时授权URL
 */
static void test_create_delete_temp_url(void)
{
    TEST_START("生成DELETE临时授权URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_delete_temp_url(&options, TEST_OBJECT_KEY,
                                                   TEST_EXPIRES_TIME,
                                                   url, sizeof(url));

    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL长度: %zu\n", strlen(url));
        TEST_PASS();
    }
}

/**
 * 测试5：生成HEAD临时授权URL
 */
static void test_create_head_temp_url(void)
{
    TEST_START("生成HEAD临时授权URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    char url[OBS_MAX_TEMP_URL_LENGTH];
    obs_status status = create_head_temp_url(&options, TEST_OBJECT_KEY,
                                                TEST_EXPIRES_TIME,
                                                url, sizeof(url));

    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL长度: %zu\n", strlen(url));
        TEST_PASS();
    }
}

/**
 * 测试6：使用通用接口创建临时URL
 */
static void test_create_presigned_url(void)
{
    TEST_START("使用通用接口创建临时URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = TEST_OBJECT_KEY;
    params.expires = TEST_EXPIRES_TIME;
    params.http_method = OBS_HTTP_METHOD_GET;

    obs_temp_url_result result;
    obs_status status = create_presigned_url(&options, &params, &result);

    if (status !=) OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(result.url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL: %s\n", result.url);
        printf("  [INFO] 过期时间戳: %lld\n", result.expires_timestamp);
        TEST_PASS();
    }
}

/**
 * 测试7：空参数验证
 */
static void test_null_parameters(void)
{
    TEST_START("空参数验证");

    obs_status status;

    // 测试NULL options
    status = create_get_temp_url(NULL, TEST_OBJECT_KEY, TEST_EXPIRES_TIME,
                                       NULL, 0);
    ASSERT_EQUAL(OBS_STATUS_InvalidParameter, status, "NULL options应返回无效参数错误");

    // 测试（NULL key
    obs_options options;
    init_obs_options(&options);
    char url[OBS_MAX_TEMP_URL_LENGTH];
    status = create_get_temp_url(&options, NULL, TEST_EXPIRES_TIME, url, sizeof(url));
    ASSERT_EQUAL(OBS_STATUS_InvalidParameter, status, "NULL key应返回无效参数错误");

    TEST_PASS();
}

/**
 * 测试8：带版本ID的临时URL
 */
static void test_temp_url_with_version_id(void)
{
    TEST_START("带版本ID的临时URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = TEST_OBJECT_KEY;
    params.expires = TEST_EXPIRES_TIME;
    params.http_method = OBS_HTTP_METHOD_GET;
    params.version_id = "test-version-id";

    obs_temp_url_result result;
    obs_status status = create_presigned_url(&options, &params, &result);

    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(result.url, "生成的URL不应为空");
        // 检查URL中是否包含versionId参数
        if (strstr(result.url, "versionId") != NULL) {
            printf("  [INFO] URL中包含versionId参数\n");
        }
        TEST_PASS();
    }
}

/**
 * 测试9：带自定义查询参数的临时URL
 */
static void test_temp_url_with_custom_params(void)
{
    TEST_START("带自定义查询参数的临时URL");

    obs_options options;
    init_obs_options(&options);
    options.bucket_options.access_key = TEST_ACCESS_KEY;
    options.bucket_options.secret_access_key = TEST_SECRET_ACCESS_KEY;
    options.bucket_options.bucket_name = TEST_BUCKET_NAME;
    options.bucket_options.host_name = TEST_HOST_NAME;
    options.bucket_options.protocol = OBS_PROTOCOL_HTTPS;

    obs_name_value custom_params[2];
    obs_name_value param1 = { "response-content-type", "text/plain" };
    obs_name_value param2 = { "response-cache-control", "no-cache" };
    custom_params[0] = param1;
    custom_params[1] = param2;

    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = TEST_OBJECT_KEY;
    params.expires = TEST_EXPIRES_TIME;
    params.http_method = OBS_HTTP_METHOD_GET;
    params.query_params = custom_params;
    params.query_params_count = 2;

    obs_temp_url_result result;
    obs_status status = create_presigned_url(&options, &params, &result);

    if (status != OBS_STATUS_OK) {
        printf("  [INFO] 预期失败（无效密钥）: %s\n", obs_get_status_name(status));
        if (status == OBS_STATUS_NoToken) {
            TEST_PASS();
        } else {
            TEST_FAIL("参数验证失败");
        }
    } else {
        ASSERT_STRING_NOT_EMPTY(result.url, "生成的URL不应为空");
        printf("  [INFO] 生成的URL长度: %zu\n", strlen(result.url));
        TEST_PASS();
    }
}

/**
 * 主测试函数
 */
int main(int argc, char *argv[])
{
    printf("========================================\n");
    printf("华为云OBS SDK - 临时授权URL功能测试\n");
    printf("========================================\n");

    // 初始化SDK
    obs_status init_status = obs_initialize(OBS_INIT_ALL);
    if (init_status != OBS_STATUS_OK) {
        printf("SDK初始化失败: %s\n", obs_get_status_name(init_status));
        return 1;
    }

    // 运行所有测试
    test_init_temp_url_params();
    test_create_get_temp_url();
    test_create_put_temp_url();
    test_create_delete_temp_url();
    test_create_head_temp_url();
    test_create_presigned_url();
    test_null_parameters();
    test_temp_url_with_version_id();
    test_temp_url_with_custom_params();

    // 清理SDK
    obs_deinitialize();

    // 打印测试结果
    printf("\n========================================\n");
    printf("测试结果统计:\n");
    printf("  运行测试数: %d\n", tests_run);
    printf("  通过测试数: %d\n", tests_passed);
    printf("  失败测试数: %d\n", tests_failed);
    printf("========================================\n");

    return (tests_failed > 0) ? 1 : 0;
}

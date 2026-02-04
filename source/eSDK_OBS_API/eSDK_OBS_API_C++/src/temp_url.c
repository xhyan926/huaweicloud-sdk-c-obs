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
#include <time.h>
#include "temp_url.h"
#include "util.h"
#include "log.h"
#include "securec.h"

/**
 * HTTP方法字符串映射
 */
static const char *g_http_method_strings[] = {
    "GET",    // OBS_HTTP_METHOD_GET
    "PUT",    // OBS_HTTP_METHOD_PUT
    "DELETE",  // OBS_HTTP_METHOD_DELETE
    "HEAD",    // OBS_HTTP_METHOD_HEAD
    "POST"     // OBS_HTTP_METHOD_POST
};

/**
 * 初始化临时授权URL参数结构
 */
void init_temp_url_params(obs_temp_url_params *params)
{
    if (params == NULL) {
        return;
    }

    memset_s(params, sizeof(obs_temp_url_params), 0, sizeof(obs_temp_url_params));
    params->http_method = OBS_HTTP_METHOD_GET;
    params->expires = 3600; // 默认1小时
}

/**
 * URL编码（复用util.h中的urlEncode）
 */
static int url_encode_param(char *dest, const char *src, int max_dest_size)
{
    return urlEncode(dest, src, strlen(src), max_dest_size, '\0');
}

/**
 * 构建规范化资源路径
 */
static obs_status build_canonicalized_resource(
    char *output,
    int output_size,
    const char *bucket_name,
    const char *key)
{
    int len = 0;

    // 斜杠
    if (len + 1 >= output_size) {
        return OBS_STATUS_UriTooLong;
    }
    len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "/");

    // 桶名
    if (bucket_name != NULL && bucket_name[0] != '\0') {
        if (len + strlen(bucket_name) >= output_size) {
            return OBS_STATUS_UriTooLong;
        }
        len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "%s", bucket_name);

        // 如果有key，添加斜杠
        if (key != NULL && key[0] != '\0') {
            if (len + 1 >= output_size) {
                return OBS_STATUS_UriTooLong;
            }
            len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "/");

            // URL编码的key
            int encoded_len = url_encode_param(&output[len], key, output_size - len);
            if (encoded_len < 0) {
                return OBS_STATUS_UriTooLong;
            }
            len += encoded_len;
        }
    }

    return OBS;
_STATUS_OK;
}

/**
 * 构建规范化查询字符串
 */
static obs_status build_canonicalized_query_string(
    char *output,
    int output_size,
    const obs_name_value *query_params,
    unsigned int query_params_count,
    uint64_t expires,
    const char *version_id)
{
    int len = 0;

    // 添加过期时间参数
    if (len + 100 >= output_size) {
        return OBS_STATUS_QueryParamsTooLong;
    }
    len += snprintf_s(&output[len], output_size - len, _TRUNCATE,
                     "Expires=%llu", (unsigned long long)expires);

    // 添加版本ID参数
    if (version_id != NULL && version_id[0] != '\0') {
        char encoded_version[MAX_URLENCODED_KEY_SIZE + 1];
        int encoded_len = url_encode_param(encoded_version, version_id, sizeof(encoded_version));
        if (encoded_len < 0) {
            return OBS_STATUS_QueryParamsTooLong;
        }

        if (len + encoded_len + 50 >= output_size) {
            return OBS_STATUS_QueryParamsTooLong;
        }
        len += snprintf_s(&output[len], output_size - len, _TRUNCATE,
                         "&versionId=%s", encoded_version);
    }

    // 添加自定义查询参数
    if (query_params != NULL && query_params_count > 0) {
        for (unsigned int i = 0; i < query_params_count; i++) {
            if (query_params[i].name != NULL && query_params[i].value != NULL) {
                char encoded_name[MAX_URLENCODED_KEY_SIZE + 1];
                char encoded_value[MAX_URLENCODED_KEY_SIZE + 1];

                int name_len = url_encode_param(encoded_name, query_params[i].name, sizeof(encoded_name));
                int value_len = url_encode_param(encoded_value, query_params[i].value, sizeof(encoded_value));

                if (name_len < 0 || value_len < 0) {
                    return OBS_STATUS_QueryParamsTooLong;
                }

                if (len + name_len + value_len + 10 >= output_size) {
                    return OBS_STATUS_QueryParamsTooLong;
                }
                len += snprintf_s(&output[len], output_size - len, _TRUNCATE,
                                 "&%s=%s", encoded_name, encoded_value);
            }
        }
    }

    return OBS_STATUS_OK;
}

/**
 * 获取当前时间戳（ISO 8601格式）
 */
static void get_iso8601_time(char *buffer, int buffer_size)
{
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);

    if (tm_info != NULL) {
        strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%SZ", GMTIME_INFO_TM);
    }
}

/**
 * 生成V2签名（华为云OBS使用）
 */
static obs_status generate_signature_v2(
    const char *secret_access_key,
    const char *string_to_sign,
    char *signature_out,
    int signature_out_size)
{
    unsigned char hmac[20];
    char base64_signature[64];

    // 计算HMAC-SHA1
    HMAC_SHA1(hmac,
               (const unsigned char *)secret_access_key,
               strlen(secret_access_key),
               (const unsigned char *)string_to_sign,
               strlen(string_to_sign));

    // Base64编码
    int result = base64Encode(hmac, 20, base64_signature);
    if (result < 0) {
        return OBS_STATUS_BadDigest;
    }

    // 复制到输出（移除可能存在的换行符）
    int len = 0;
    for (int i = 0; i < result && len < signature_out_size - 1; i++) {
        if (base64_signature[i] != '\n' && base64_signature[i] != '\r') {
            signature_out[len++] = base64_signature[i];
        }
    }
    signature_out[len] = '\0';

    return OBS_STATUS_OK;
}

/**
 * 构建待签名字符串（OBS V2签名）
 */
static obs_status build_string_to_sign_v2(
    char *output,
    int output_size,
    const char *http_method,
    const char *expires,
    const char *canonicalized_resource)
{
    int len = 0;

    // HTTP方法 + \n
    if (len + strlen(http_method) + 1 >= output_size) {
        return OBS_STATUS_XmlDocumentTooLarge;
    }
    len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "%s\n", http_method);

    // Expires + \n
    if (len + strlen(expires) + 1 >= output_size) {
        return OBS_STATUS_XmlDocumentTooLarge;
    }
    len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "%s\n", expires);

    // CanonicalizedResource
    if (len + strlen(canonicalized_resource) >= output_size) {
        return OBS_STATUS_XmlDocumentTooLarge;
    }
    len += snprintf_s(&output[len], output_size - len, _TRUNCATE, "%s", canonicalized_resource);

    return OBS_STATUS_OK;
}

/**
 * 构建完整URL
 */
static obs_status build_full_url(
    char *url_out,
    int url_out_len,
    const obs_bucket_context *bucket_context,
    const char *canonicalized_resource,
    const char *canonicalized_query_string,
    const char *access_key,
    const char *signature)
{
    int len = 0;
    const char *protocol = (bucket_context->protocol == OBS_PROTOCOL_HTTPS) ? "https://" : "http://";

    // 协议和主机名
    if (len + strlen(protocol) + strlen(bucket_context->host_name) >= url_out_len) {
        return OBS_STATUS_UriTooLong;
    }
    len += snprintf_s(&url_out[len], url_out_len - len, _TRUNCATE,
                     "%s%s", protocol, bucket_context->host_name);

    // 规范化资源路径
    if (len + strlen(canonicalized_resource) >= url_out_len) {
        return OBS_STATUS_UriTooLong;
    }
    len += snprintf_s(&url_out[len], url_out_len - len, _TRUNCATE,
                     "%s", canonicalized_resource);

    // 查询字符串
    if (canonicalized_query_string != NULL && canonicalized_query_string[0] != '\0') {
        if (len + strlen(canonicalized_query_string) + 1 >= url_out_len) {
            return OBS_STATUS_UriTooLong;
        }
        len += snprintf_s(&url_out[len], url_out_len - len, _TRUNCATE,
                         "?%s", canonicalized_query_string);
    }

    // AWSAccessKeyId
    if (len + strlen(access_key) + 50 >= url_out_len) {
        return OBS_STATUS_UriTooLong;
    }
    len += snprintf_s(&url_out[len], url_out_len - len, _TRUNCATE,
                     "&AWSAccessKeyId=%s", access_key);

    // Signature
    char encoded_signature[128];
    int sig_len = url_encode_param(encoded_signature, signature, sizeof(encoded_signature));
    if (sig_len < 0) {
        return OBS_STATUS_UriTooLong;
    }

    if (len + sig_len + 20 >= url_out_len) {
        return OBS_STATUS_UriTooLong;
    }
    len += snprintf_s(&url_out[len], url_out_len - len, _TRUNCATE,
                     "&Signature=%s", encoded_signature);

    return OBS_STATUS_OK;
}

/**
 * 生成通用临时授权URL（核心实现）
 */
static obs_status create_presigned_url_internal(
    const obs_options *options,
    const obs_temp_url_params *params,
    char *url_out,
    int url_out_len,
    int64_t *expires_timestamp_return)
{
    obs_status status;

    // 参数验证
    if (options == NULL || params == NULL || url_out == NULL) {
        COMMLOG(OBS_LOGERROR, "Invalid parameters in create_presigned_url");
        return OBS_STATUS_InvalidParameter;
    }

    if (params->key == NULL || params->key[0] == '\0') {
        COMMLOG(OBS_LOGERROR, "Key parameter is required");
        return OBS_STATUS_InvalidParameter;
    }

    if (params->expires == 0) {
        COMMLOG(OBS_LOGERROR, "Expires parameter is required");
        return OBS_STATUS_InvalidParameter;
    }

    const obs_bucket_context *bucket_ctx = &options->bucket_options;
    if (bucket_ctx->access_key == NULL || bucket_ctx->secret_access_key == NULL) {
        COMMLOG(OBS_LOGERROR, "Access key and secret access key are required");
        return OBS_STATUS_NoToken;
    }

    // 计算过期时间戳
    time_t now = time(NULL);
    uint64_t expires_timestamp = (uint64_t)now + params->expires;

    if (expires_timestamp_return != NULL) {
        *expires_timestamp_return = (int64_t)expires_timestamp;
    }

    // HTTP方法字符串
    if (params->http_method < 0 || params->http_method >= sizeof(g_http_method_strings) / sizeof(g_http_method_strings[0])) {
        COMMLOG(OBS_LOGERROR, "Invalid HTTP method");
        return OBS_STATUS_InvalidParameter;
    }
    const char *http_method_str = g_http_method_strings[params->http_method];

    // 构建规范化资源路径
    char canonicalized_resource[MAX_CANONICALIZED_RESOURCE_SIZE + 1];
    status = build_canonicalized_resource(canonicalized_resource,
                                      sizeof(canonicalized_resource),
                                      bucket_ctx->bucket_name,
                                      params->key);
    if (status != OBS_STATUS_OK) {
        COMMLOG(OBS_LOGERROR, "Failed to build canonicalized resource: %d", status);
        return status;
    }

    // 构建规范化查询字符串
    char canonicalized_query[OBS_MAX_QUERY_STRING_SIZE + 1];
    status = build_canonicalized_query_string(canonicalized_query,
                                          sizeof(canonicalized_query),
                                          params->query_params,
                                          params->query_params_count,
                                          expires_timestamp,
                                          params->version_id);
    if (status != OBS_STATUS_OK) {
        COMMLOG(OBS_LOGERROR, "Failed to build canonicalized query string: %d", status);
        return status;
    }

    // 构建待签名字符串
    char string_to_sign[4096];
    status = build_string_to_sign_v2(string_to_sign,
                                     sizeof(string_to_sign),
                                     http_method_str,
                                     canonicalized_query,
                                     canonicalized_resource);
    if (status != OBS_STATUS_OK) {
        COMMLOG(OBS_LOGERROR, "Failed to build string to sign: %d", status);
        return status;
    }

    // 生成签名
    char signature[128];
    status = generate_signature_v2(bucket_ctx->secret_access_key,
                                  string_to_sign,
                                  signature,
                                  sizeof(signature));
    if (status != OBS_STATUS_OK) {
        COMMLOG(OBS_LOGERROR, "Failed to generate signature: %d", status);
        return status;
    }

    // 构建完整URL
    status = build_full_url(url_out, url_out_len, bucket_ctx,
                           canonicalized_resource,
                           canonicalized_query,
                           bucket_ctx->access_key,
                           signature);
    if (status != OBS_STATUS_OK) {
        COMMLOG(OBS_LOGERROR, "Failed to build full URL: %d", status);
        return status;
    }

    COMMLOG(OBS_LOGINFO, "Successfully created presigned URL for method: %s, key: %s",
             http_method_str, params->key);

    return OBS_STATUS_OK;
}

/**
 * 生成通用临时授权URL（公开接口）
 */
obs_status create_presigned_url(
    const obs_options *options,
    const obs_temp_url_params *params,
    obs_temp_url_result *result)
{
    if (result == NULL) {
        return OBS_STATUS_InvalidParameter;
    }

    obs_status status = create_presigned_url_internal(
        options,
        params,
        result->url,
        sizeof(result->url),
        &result->expires_timestamp
    );

    result->status = status;
    result->error_message = (status == OBS_STATUS_OK) ? NULL : obs_get_status_name(status);

    return status;
}

/**
 * 生成GET临时授权URL
 */
obs_status create_get_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len)
{
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = key;
    params.expires = expires;
    params.http_method = OBS_HTTP_METHOD_GET;

    return create_presigned_url_internal(options, &params, url_out, url_out_len, NULL);
}

/**
 * 生成PUT临时授权URL
 */
obs_status create_put_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    const char *content_type,
    char *url_out,
    int url_out_len)
{
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = key;
    params.expires = expires;
    params.http_method = OBS_HTTP_METHOD_PUT;

    // 如果需要指定content-type，可以通过query_params添加
    if (content_type != NULL && content_type[0] != '\0') {
        obs_name_value ct_param;
        ct_param.name = "Content-Type";
        ct_param.value = content_type;
        params.query_params = &ct_param;
        params.query_params_count = 1;
    }

    return create_presigned_url_internal(options, &params, url_out, url_out_len, NULL);
}

/**
 * 生成DELETE临时授权URL
 */
obs_status create_delete_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len)
{
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = key;
    params.expires = expires;
    params.http_method = OBS_HTTP_METHOD_DELETE;

    return create_presigned_url_internal(options, &params, url_out, url_out_len, NULL);
}

/**
 * 生成HEAD临时授权URL
 */
obs_status create_head_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len)
{
    obs_temp_url_params params;
    init_temp_url_params(&params);
    params.key = key;
    params.expires = expires;
    params.http_method = OBS_HTTP_METHOD_HEAD;

    return create_presigned_url_internal(options, &params, url_out, url_out_len, NULL);
}

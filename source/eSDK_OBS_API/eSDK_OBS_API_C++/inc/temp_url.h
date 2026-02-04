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
**********************************************************************************
*/
#ifndef TEMP_URL_H
#define TEMP_URL_H

#include "eSDKOBS.h"

#define OBS_MAX_TEMP_URL_LENGTH  4096
#define OBS_MAX_QUERY_STRING_SIZE 1024

/**
 * HTTP方法枚举，用于临时授权URL
 */
typedef enum
{
    OBS_HTTP_METHOD_GET = 0,
    OBS_HTTP_METHOD_PUT = 1,
    OBS_HTTP_METHOD_DELETE = 2,
    OBS_HTTP_METHOD_HEAD = 3,
    OBS_HTTP_METHOD_POST = 4
} obs_http_method;

/**
 * 临时授权URL参数
 */
typedef struct obs_temp_url_params
{
    const char *key;                          // 对象键名，必填
    const char *version_id;                    // 版本ID（可选），用于访问特定版本的对象
    uint64_t expires;                        // 过期时间（秒），必填，建议值：7天=604800秒
    obs_http_method http_method;                // HTTP方法，必填
    obs_name_value *query_params;              // 查询参数（可选）
    unsigned int query_params_count;            // 查询参数数量
    const char *response_content_type;          // 响应内容类型（可选）
    const char *response_content_language;      // 响应内容语言（可选）
    const char *response_cache_control;         // 响应缓存控制（可选）
    const char *response_content_disposition;   // 响应内容处置（可选）
    const char *response_content_encoding;      // 响应内容编码（可选）
    const char *response_expires;              // 响应过期时间（可选）
} obs_temp_url_params;

/**
 * 临时授权URL生成结果
 */
typedef struct obs_temp_url_result
{
    char url[OBS_MAX_TEMP_URL_LENGTH];        // 生成的临时URL
    int64_t expires_timestamp;                // 过期时间戳
    obs_status status;                       // 生成状态
    const char *error_message;                // 错误消息（如果失败）
} obs_temp_url_result;

/**
 * 初始化临时授权URL参数结构
 */
void init_temp_url_params(obs_temp_url_params *params);

/**
 * 生成通用临时授权URL
 *
 * @param options SDK配置选项
 * @param params 临时授权URL参数
 * @param result 输出结果，包含生成的URL
 * @return OBS_STATUS_OK成功，其他失败
 */
obs_status create_presigned_url(
    const obs_options *options,
    const obs_temp_url_params *params,
    obs_temp_url_result *result
);

/**
 * 生成GET临时授权URL（用于下载对象）
 *
 * @param options SDK配置选项
 * @param key 对象键名
 * @param expires 过期时间（秒）
 * @param url_out 输出URL缓冲区
 * @param url_out_len URL缓冲区大小
 * @return OBS_STATUS_OK成功，其他失败
 */
obs_status create_get_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);

/**
 * 生成PUT临时授权URL（用于上传对象）
 *
 * @param options SDK配置选项
 * @param key 对象键名
 * @param expires 过期时间（秒）
 * @param content_type 内容类型（可选）
 * @param url_out 输出URL缓冲区
 * @param url_out_len URL缓冲区大小
 * @return OBS_STATUS_OK成功，其他失败
 */
obs_status create_put_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    const char *content_type,
    char *url_out,
    int url_out_len
);

/**
 * 生成DELETE临时授权URL（用于删除对象）
 *
 * @param options SDK配置选项
 * @param key 对象键名
 * @param expires 过期时间（秒）
 * @param url_out 输出URL缓冲区
 * @param url_out_len URL缓冲区大小
 * @return OBS_STATUS_OK成功，其他失败
 */
obs_status create_delete_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);

/**
 * 生成HEAD临时授权URL（用于获取对象元数据）
 *
 * @param options SDK配置选项
 * @param key 对象键名
 * @param expires 过期时间（秒）
 * @param url_out 输出URL缓冲区
 * @param url_out_len URL缓冲区大小
 * @return OBS_STATUS_OK成功，其他失败
 */
obs_status create_head_temp_url(
    const obs_options *options,
    const char *key,
    uint64_t expires,
    char *url_out,
    int url_out_len
);

#endif /* TEMP_URL_H */

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
#include "eSDKOBS.h"
#include "request.h"
#include "simplexml.h"
#include "util.h"
#include "log.h"
#include "securec.h"
#include "common.h"

#define MAX_ENCRYPTION_DOC_SIZE (2 * 1024)

/**
 * 桶加密配置数据结构（用于内部处理）
 */
typedef struct bucket_encryption_data
{
    simple_xml simpleXml;

    obs_response_properties_callback *responsePropertiesCallback;
    obs_response_complete_callback *responseCompleteCallback;
    obs_get_bucket_encryption_callback *getEncryptionCallback;
    void *callback_data;

    obs_bucket_encryption_config *configReturn;

    string_buffer(encryption_algorithm, 32);
    string_buffer(kms_key_id, 256);
    obs_use_api use_api;
} bucket_encryption_data;

/**
 * 桶加密配置数据结构（用于设置/删除）
 */
typedef struct set_bucket_encryption_data
{
    obs_response_properties_callback *responsePropertiesCallback;
    obs_response_complete_callback *responseCompleteCallback;
    void *callback_data;

    char doc[MAX_ENCRYPTION_DOC_SIZE];
    int docLen;
    int docBytesWritten;
} set_bucket_encryption_data;

/**
 * 追加XML文档
 */
static obs_status append_encryption_document(int *doc_len_return, char *doc,
                                       int doc_buffer_size, char *fmt, ...)
{
    int len = *doc_len_return;

    if (len >= doc_buffer_size) {
        COMMLOG(OBS_LOGERROR, "Encryption document buffer too small in function: %s, line: %d",
                 __FUNCTION__, __LINE__);
        return OBS_STATUS_XmlDocumentTooLarge;
    }

    // Format the new string into the buffer
    len += vsnprintf_s(&(doc[len]), doc_buffer_size - len, fmt,
                     (char *)(&fmt) + sizeof(fmt));

    if (len >= doc_buffer_size) {
        COMMLOG(OBS_LOGERROR, "Encryption document buffer overflow in function: %s, line: %d",
                 __FUNCTION__, __LINE__);
        return OBS_STATUS_XmlDocumentTooLarge;
    }

    *doc_len_return = len;
    return OBS_STATUS_OK;
}

/**
 * 生成设置桶加密配置的XML文档
 */
static obs_status generate_set_encryption_xml(const obs_bucket_encryption_config *config,
                                         set_bucket_encryption_data *data)
{
    obs_status status = OBS_STATUS_OK;

    memset_s(data->doc, 0, sizeof(data->doc));
    data->docLen = 0;
    data->docBytesWritten = 0;

    // XML header
    status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                   "%s", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    if (status != OBS_STATUS_OK) return status;

    status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                   "%s", "<ServerSideEncryptionConfiguration xmlns=\"http://obs.myhuaweicloud.com/doc/2015-06-30/\">\n");
    if (status != OBS_STATUS_OK) return status;

    // 根据加密类型生成对应的配置
    switch (config->encryption_type) {
        case OBS_BUCKET_ENCRYPTION_KMS:
            // SSE-KMS配置
            if (config->kms_key_id != NULL && config->kms_key_id[0] != '\0') {
                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "  <Rule>\n");
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "    <ApplyServerSideEncryptionByDefault>\n");
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "      <SSE-KMS>\n");
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "        <KMSKeyId>%s</KMSKeyId>\n",
                                               config->kms_key_id);
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "      </SSE-KMS>\n");
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "    </ApplyServerSideEncryptionByDefault>\n");
                if (status != OBS_STATUS_OK) return status;

                status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                               "  </Rule>\n");
                if (status != OBS_STATUS_OK) return status;
            }
            break;

        case OBS_BUCKET_ENCRYPTION_AES256:
            // AES256配置
            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "  <Rule>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "    <ApplyServerSideEncryptionByDefault>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "      <AES256></AES256>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "    </ApplyServerSideEncryptionByDefault>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "  </Rule>\n");
            if (status != OBS_STATUS_OK) return status;
            break;

        case OBS_BUCKET_ENCRYPTION_SM4:
            // SM4配置（国密）
            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "  <Rule>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "    <ApplyServerSideEncryptionByDefault>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "      <SM4></SM4>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "    </ApplyServerSideEncryptionByDefault>\n");
            if (status != OBS_STATUS_OK) return status;

            status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                           "  </Rule>\n");
            if (status != OBS_STATUS_OK) return status;
            break;

        default:
            COMMLOG(OBS_LOGERROR, "Invalid encryption type: %d in function: %s",
                     config->encryption_type, __FUNCTION__);
            return OBS_STATUS_InvalidParameter;
    }

    // XML footer
    status = append_encryption_document(&data->docLen, data->doc, sizeof(data->doc),
                                   "%s", "</ServerSideEncryptionConfiguration>\n");
    if (status != OBS_STATUS_OK) return status;

    COMMLOG(OBS_LOGINFO, "Generated bucket encryption XML document: %s", data->doc);
    return OBS_STATUS_OK;
}

/**
 * 设置桶加密配置
 */
void set_bucket_encryption(const obs_options *options,
                          const obs_bucket_encryption_config *config,
                          obs_response_handler *handler, void *callback_data)
{
    obs_status status = check_options_and_handler_params(__FUNCTION__, options, handler, callback_data);
    if (status != OBS_STATUS_OK) {
        return_status(status);
    }

    if (config == NULL) {
        COMMLOG(OBS_LOGERROR, "Invalid parameter: config is NULL in function: %s", __FUNCTION__);
        return_status(OBS_STATUS_InvalidParameter);
    }

    // 设置加密配置数据
    set_bucket_encryption_data data;
    memset_s(&data, 0, sizeof(data));
    data.responsePropertiesCallback = handler->properties_callback;
    data.responseCompleteCallback = handler->complete_callback;
    data.callback_data = callback_data;

    // 生成XML文档
    status = generate_set_encryption_xml(config, &data);
    if (status != OBS_STATUS_OK) {
        return_status(status);
    }

    // 设置请求参数
    request_params params;
    memset_s(&params, 0, sizeof(params));
    params.httpRequestType = http_request_type_put;
    params.bucketContext = options->bucket_options;
    params.subResource = "encryption";
    params.properties_callback = handler->properties_callback;
    params.fromObsCallback = NULL;
    params.toObsCallback = NULL;
    params.complete_callback = handler->complete_callback;
    params.callback_data = callback_data;
    params.toObsCallbackTotalSize = data.docLen;

    // 发送请求
    request_perform(&params);
}

/**
 * 解析获取桶加密配置的XML响应
 */
static obs_status parse_get_encryption_xml_callback(int buffer_size, const char *buffer,
                                                  void *callback_data)
{
    bucket_encryption_data *data = (bucket_encryption_data *)callback_data;

    return simple_xml_parse(&(data->simpleXml), buffer_size, buffer);
}

/**
 * 属性回调（获取加密配置）
 */
static obs_status get_encryption_properties_callback(const obs_response_properties *response_properties,
                                                  void *callback_data)
{
    bucket_encryption_data *data = (bucket_encryption_data *)callback_data;

    if (data->responsePropertiesCallback) {
        (*(data->responsePropertiesCallback))(response_properties, data->callback_data);
    }

    return OBS_STATUS_OK;
}

/**
 * 完成回调（获取加密配置）
 */
static void get_encryption_complete_callback(obs_status status,
                                        const obs_error_details *error_details,
                                        void *callback_data)
{
    bucket_encryption_data *data = (bucket_encryption_data *)callback_data;

    if (status != OBS_STATUS_OK) {
        if (data->responseCompleteCallback) {
            (*(data->responseCompleteCallback))(status, error_details, data->callback_data);
        }
        return;
    }

    // 调用用户的回调
    if (data->getEncryptionCallback && data->configReturn) {
        obs_status callback_status = (*(data->getEncryptionCallback))(data->configReturn,
                                                                   data->callback_data);
        if (callback_status != OBS_STATUS_OK) {
            status = callback_status;
        }
    }

    if (data->responseCompleteCallback) {
        (*(data->responseCompleteCallback))(status, error_details, data->callback_data);
    }
}

/**
 * 获取桶加密配置
 */
void get_bucket_encryption(const obs_options *options,
                          obs_get_bucket_encryption_handler *handler,
                          void *callback_data)
{
    obs_status status = check_options_and_handler_params(__FUNCTION__, options,
                                                          &handler->response_handler, callback_data);
    if (status != OBS_STATUS_OK) {
        return;
    }

    // 设置加密配置数据
    bucket_encryption_data data;
    memset_s(&data, 0, sizeof(data));
    data.responsePropertiesCallback = handler->response_handler.properties_callback;
    data.responseCompleteCallback = handler->response_handler.complete_callback;
    data.getEncryptionCallback = handler->get_encryption_callback;
    data.callback_data = callback_data;

    // 初始化配置返回结构
    obs_bucket_encryption_config config;
    memset_s(&config, 0, sizeof(config));
    data.configReturn = &config;

    // 获取API版本
    status = get_api_version(options->bucket_options.bucket_name,
                           options->bucket_options.host_name,
                           options->bucket_options.protocol,
                           options->http_request_option,
                           options->bucket_options.is_cname);

    if (status == OBS_STATUS_OK || status == OBS_STATUS_UseApiS3) {
        data.use_api = OBS_USE_API_TYPE_S3;
    } else {
        data.use_api = OBS_USE_API_TYPE_OBS;
    }

    // 设置请求参数
    request_params params;
    memset_s(&params, 0, sizeof(params));
    params.httpRequestType = http_request_type_get;
    params.bucketContext = options->bucket;
    params.subResource = "encryption";
    params.properties_callback = get_encryption_properties_callback;
    params.fromObsCallback = parse_get_encryption_xml_callback;
    params.complete_callback = get_encryption_complete_callback;
    params.callback_data = &data;

    // 初始化XML解析器
    simple_xml_initialize(&(data.simpleXml),
                          get_encryption_properties_callback,
                          get_encryption_complete_callback,
                          &data);

    // 发送请求
    request_perform(&params);

    // 清理XML解析器
    simple_xml_deinitialize(&(data.simpleXml));
}

/**
 * 删除桶加密配置
 */
void delete_bucket_encryption(const obs_options *options,
                             obs_response_handler *handler, void *callback_data)
{
    obs_status status = check_options_and_handler_params(__FUNCTION__, options, handler, callback_data);
    if (status != OBS_STATUS_OK) {
        return;
    }

    // 设置请求参数
    request_params params;
    memset_s(&params, 0, sizeof(params));
    params.httpRequestType = http_request_type_put;
    params.bucketContext = options->bucket_options;
    params.subResource = "encryption";
    params.properties_callback = handler->properties_callback;
    params.fromObsCallback = NULL;
    params.toObsCallback = NULL;
    params.complete_callback = handler->complete_callback;
    params.callback_data = callback_data;

    // 删除加密配置：发送空的XML文档
    set_bucket_encryption_data data;
    memset_s(&data, 0, sizeof(data));
    data.responsePropertiesCallback = handler->properties_callback;
    data.responseCompleteCallback = handler->complete_callback;
    data.callback_data = callback_data;

    // 生成空配置XML（删除操作）
    snprintf_s(data.doc, sizeof(data.doc), _TRUNCATE,
             "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<ServerSideEncryptionConfiguration xmlns=\"http://obs.myhuaweicloud.com/doc/2015-06-30/\">\n"
             "</ServerSideEncryptionConfiguration>\n");
    data.docLen = strlen(data.doc);
    data.toObsCallbackTotalSize = data.docLen;

    // 发送请求
    request_perform(&params);
}

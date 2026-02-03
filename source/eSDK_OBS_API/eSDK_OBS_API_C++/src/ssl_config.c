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
#include "ssl_config.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include "securec.h"

#define MAX_CONFIG_LINE 256
#define CONFIG_FILE "OBS.ini"

// 辅助函数：解析配置项字符串值
static void parse_config_string_value(const char *line, char *buffer, size_t buffer_size)
{
    char *value = strchr(line, '=');
    if (value && *(value + 1))
    {
        value++;
        size_t len = strlen(value);
        if (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r'))
        {
            value[len - 1] = '\0';
            len--;
        }
        if (len > 0 && len < buffer_size)
        {
            errno_t err = strcpy_s(buffer, buffer_size, value, len);
            if (err != EOK)
            {
                COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
            }
        }
    }
}

// 辅助函数：分配并复制字符串
static void alloc_copy_string(const char *src, char **dest)
{
    if (src && src[0] != '\0')
    {
        size_t len = strlen(src);
        *dest = (char *)malloc(len + 1);
        if (*dest)
        {
            errno_t err = strcpy_s(*dest, len + 1, src, len);
            if (err != EOK)
            {
                COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
            }
            COMMLOG(OBS_LOGINFO, "%s Config value: %s", __FUNCTION__, src);
        }
    }
}

// 辅助函数：解析SSL版本
static void parse_ssl_version(const char *ver_str, long *dest, long default_value)
{
    if (ver_str && ver_str[0] != '\0')
    {
        if (strcmp(ver_str, "1.0") == 0)
            *dest = CURL_SSLVERSION_TLSv1_0;
        else if (strcmp(ver_str, "1.1") == 0)
            *dest = CURL_SSLVERSION_TLSv1_1;
        else if (strcmp(ver_str, "1.2") == 0)
            *dest = CURL_SSLVERSION_TLSv1_2;
        else if (strcmp(ver_str, "1.3") == 0)
            *dest = (1 << 16) | 3;  // CURL_SSLVERSION_TLSv1_3
        else
            *dest = default_value;

        COMMLOG(OBS_LOGINFO, "%s SSL version: %s", __FUNCTION__, ver_str);
    }
}

void load_ssl_config_from_ini(obs_options *options)
{
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp)
    {
        COMMLOG(OBS_LOGWARN, "%s Config file not found: %s", __FUNCTION__, CONFIG_FILE);
        return;
    }

    char line[MAX_CONFIG_LINE] = {0};
    int in_ssl_config = 0;

    char mutual_ssl_str[16] = {0};
    char client_cert_path[1024] = {0};
    char client_key_path[1024] = {0};
    char client_key_password[64] = {0};
    char gm_mode_str[16] = {0};
    char ssl_cipher_list[512] = {0};
    char ssl_min_ver_str[16] = {0};
    char ssl_max_ver_str[16] = {0};

    while (fgets(line, sizeof(line), fp))
    {
        // 跳过空行和注释
        if (line[0] == '\n' || line[0] == '#' || line[0] == ';' || line[0] == '\r')
        {
            continue;
        }

        // 检测 [SSLConfig] 段
        if (strstr(line, "[SSLConfig]"))
        {
            in_ssl_config = 1;
            continue;
        }
        else if (line[0] == '[')
        {
            in_ssl_config = 0;
            continue;
        }

        if (!in_ssl_config)
        {
            continue;
        }

        // 解析配置项
        if (strstr(line, "MutualSSLEnabled"))
        {
            sscanf(line, "MutualSSLEnabled=%15s", mutual_ssl_str);
        }
        else if (strstr(line, "ClientCertPath"))
        {
            parse_config_string_value(line, client_cert_path, sizeof(client_cert_path));
        }
        else if (strstr(line, "ClientKeyPath"))
        {
            parse_config_string_value(line, client_key_path, sizeof(client_key_path));
        }
        else if (strstr(line, "ClientKeyPassword"))
        {
            parse_config_string_value(line, client_key_password, sizeof(client_key_password));
        }
        else if (strstr(line, "GMModeEnabled"))
        {
            sscanf(line, "GMModeEnabled=%15s", gm_mode_str);
        }
        else if (strstr(line, "CipherList"))
        {
            parse_config_string_value(line, ssl_cipher_list, sizeof(ssl_cipher_list));
        }
        else if (strstr(line, "SSLMinVersion"))
        {
            parse_config_string_value(line, ssl_min_ver_str, sizeof(ssl_min_ver_str));
        }
        else if (strstr(line, "SSLMaxVersion"))
        {
            parse_config_string_value(line, ssl_max_ver_str, sizeof(ssl_max_ver_str));
        }
    }

    fclose(fp);

    // 应用双向SSL配置
    if (strcmp(mutual_ssl_str, "true") == 0)
    {
        options->request_options.mutual_ssl_switch = OBS_MUTUAL_SSL_OPEN;
        COMMLOG(OBS_LOGINFO, "%s Mutual SSL enabled from config", __FUNCTION__);
    }

    alloc_copy_string(client_cert_path, &options->request_options.client_cert_path);
    alloc_copy_string(client_key_path, &options->request_options.client_key_path);
    alloc_copy_string(client_key_password, &options->request_options.client_key_password);

    // 应用国密模式配置
    if (strcmp(gm_mode_str, "true") == 0)
    {
        options->request_options.gm_mode_switch = OBS_GM_MODE_OPEN;
        COMMLOG(OBS_LOGINFO, "%s GM mode enabled from config", __FUNCTION__);
    }

    alloc_copy_string(ssl_cipher_list, &options->request_options.ssl_cipher_list);

    // 应用SSL版本配置
    parse_ssl_version(ssl_min_ver_str, &options->request_options.ssl_min_version, CURL_SSLVERSION_TLSv1_2);
    parse_ssl_version(ssl_max_ver_str, &options->request_options.ssl_max_version, (1 << 16) | 3);
}

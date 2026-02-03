/*********************************************************************************
* Copyright 2024 Huawei Technologies Co.,Ltd.
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use
* this file except in compliance with the License.  You may obtain a copy of the
* License at
*
* http://www.apache.com/licenses/LICENSE-2.0
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
                if (len > 0 && len < sizeof(client_cert_path))
                {
                    errno_t err = strcpy_s(client_cert_path, sizeof(client_cert_path), value, len);
                    if (err != EOK)
                    {
                        COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
                    }
                }
            }
        }
        else if (strstr(line, "ClientKeyPath"))
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
                if (len > 0 && len < sizeof(client_key_path))
                {
                    errno_t err = strcpy_s(client_key_path, sizeof(client_key_path), value, len);
                    if (err != EOK)
                    {
                        COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
                    }
                }
            }
        }
        else if (strstr(line, "ClientKeyPassword"))
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
                if (len > 0 && len < sizeof(client_key_password))
                {
                    errno_t err = strcpy_s(client_key_password, sizeof(client_key_password), value, len);
                    if (err != EOK)
                    {
                        COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
                    }
                }
            }
        }
    }

    fclose(fp);

    // 应用配置
    if (strcmp(mutual_ssl_str, "true") == 0)
    {
        options->request_options.mutual_ssl_switch = OBS_MUTUAL_SSL_OPEN;
        COMMLOG(OBS_LOGINFO, "%s Mutual SSL enabled from config", __FUNCTION__);
    }

    if (client_cert_path[0] != '\0')
    {
        size_t len = strlen(client_cert_path);
        options->request_options.client_cert_path = (char *)malloc(len + 1);
        if (options->request_options.client_cert_path)
        {
            errno_t err = strcpy_s(options->request_options.client_cert_path, len + 1, client_cert_path, len);
            if (err != EOK)
            {
                COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
            }
            COMMLOG(OBS_LOGINFO, "%s Client cert path: %s", __FUNCTION__, client_cert_path);
        }
    }

    if (client_key_path[0] != '\0')
    {
        size_t len = strlen(client_key_path);
        options->request_options.client_key_path = (char *)malloc(len + 1);
        if (options->request_options.client_key_path)
        {
            errno_t err = strcpy_s(options->request_options.client_key_path, len + 1, client_key_path, len);
            if (err != EOK)
            {
                COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
            }
            COMMLOG(OBS_LOGINFO, "%s Client key path: %s", __FUNCTION__, client_key_path);
        }
    }

    if (client_key_password[0] != '\0')
    {
        size_t len = strlen(client_key_password);
        options->request_options.client_key_password = (char *)malloc(len + 1);
        if (options->request_options.client_key_password)
        {
            errno_t err = strcpy_s(options->request_options.client_key_password, len + 1, client_key_password, len);
            if (err != EOK)
            {
                COMMLOG(OBS_LOGWARN, "%s(%d): strcpy_s failed!", __FUNCTION__, __LINE__);
            }
        }
    }
}

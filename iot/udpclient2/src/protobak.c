
/* This file is used to buid packet as defined and send to server.
 * */
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#define SEND_MAX_BUF_LEN 4096

int protoDevOnlineBuf(char *manufacture, char * moduleNumber, char *deviceId, char *deviceType, char *specialConfig, char *jsBuf);


int protoDevOnlineBuf(char *manufacture, char * moduleNumber, char *deviceId, char *deviceType, char *specialConfig, char *jsBuf)
{
    cJSON *json_root;

    /*create json string root*/
    json_root = cJSON_CreateObject();

    if (!json_root)
    {
        printf("get json_root failed\n");
        goto EXIT;
    }

    {
        cJSON * js_array, *js_body ;

        cJSON_AddStringToObject(json_root, "power", "1");
        cJSON_AddStringToObject(json_root, "name", "red");
        cJSON_AddStringToObject(json_root, "line1", "1");
        cJSON_AddStringToObject(json_root, "line2", "0");
        cJSON_AddStringToObject(json_root, "line1Display", "客厅灯");
        cJSON_AddStringToObject(json_root, "line2Display", "卧室灯");
        cJSON_AddStringToObject(json_root, "deviceType", "led");
        cJSON_AddStringToObject(json_root, "manufacture", "feixun");
        cJSON_AddStringToObject(json_root, "moduleNumber", "Flight");
        cJSON_AddStringToObject(json_root, "softwareVersion", "V1.0.0");
        cJSON_AddStringToObject(json_root, "oui", "001122");
        cJSON_AddStringToObject(json_root, "serialNumber", "1314151456");
#if 0
        cJSON_AddStringToObject(json_root, "type", "device_online");
        cJSON_AddStringToObject(json_root, "manufacture", manufacture);
        //cJSON_AddNumberToObject(json_root, "serial", serial);
        cJSON_AddStringToObject(json_root, "module_number", moduleNumber);
        cJSON_AddStringToObject(json_root, "device_id", deviceId);
        cJSON_AddStringToObject(json_root, "device_type", deviceType);
        cJSON_AddItemToObject(json_root, "special_config", js_array= cJSON_CreateArray());

        int i = 0;
        char name[2][20] = {"Test","Hello"};
        char value[2][20] = {"ongoing","what"};
        for (i = 0; i < 1; i ++)
        {            
            cJSON_AddItemToArray(js_array, js_body = cJSON_CreateObject());
            //cJSON_AddItemToObject(js_body, "name", cJSON_CreateString(name));
            //cJSON_AddItemToObject(js_body, "value", cJSON_CreateString(value));
            cJSON_AddStringToObject(js_body, "name", name[i]);
            cJSON_AddStringToObject(js_body, "value", value[i]);
        }
#endif

        {
            char *s = cJSON_PrintUnformatted(json_root);

            if (s)
            {
                strncpy(jsBuf, s, SEND_MAX_BUF_LEN - 1);
                //printf("create js_buf = %s\n", jsBuf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }

    return 0;
EXIT:
    return -1;
}

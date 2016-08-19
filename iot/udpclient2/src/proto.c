
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

        cJSON_AddStringToObject(json_root, "line1", "1");
        cJSON_AddStringToObject(json_root, "line2", "0");
        cJSON_AddStringToObject(json_root, "line1Display", "undefined");
        cJSON_AddStringToObject(json_root, "line2Display", "undefined");
        cJSON_AddStringToObject(json_root, "deviceType", "led");
        cJSON_AddStringToObject(json_root, "manufacture", "feixun");
        cJSON_AddStringToObject(json_root, "moduleNumber", "Flight");
        cJSON_AddStringToObject(json_root, "softwareVersion", "V1.0.0");
        cJSON_AddStringToObject(json_root, "oui", "001122");
        cJSON_AddStringToObject(json_root, "manufactureSN", "2ndDevice");

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

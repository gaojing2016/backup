#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "share.h"

int buildSendBuf(char *sendBuf, char *deviceSn, char *config, char *transportType);

int buildSendBuf(char *sendBuf, char *deviceSn, char *config, char *transportType)
{
    cJSON *json_root;
    /*create json string root*/
    json_root = cJSON_CreateObject();

    if (!json_root) 
    {   
        printf("get json_root faild !");
        goto EXIT;
    }   
    else 
    {   
        printf("get json_root success !");
    }   

    {   
        cJSON_AddStringToObject(json_root, "type", "devSet"); 
        cJSON_AddStringToObject(json_root, "device_sn", deviceSn); 
        cJSON_AddStringToObject(json_root, "transportType", transportType); 
        cJSON_AddStringToObject(json_root, "config", config); 

        {
            char *s = cJSON_PrintUnformatted(json_root);
            if (s) 
            {
                strncpy(sendBuf, s, MAX_SENDBUF_LEN - 1); 
                printf("sendBuf = %s\n", sendBuf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }   
    return 0;
EXIT:
    return -1; 

}

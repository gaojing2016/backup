#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mxml.h>
#include "cJSON.h"
#include "share.h"

int parseDevOnlineBuf(char *jsonBuf, char *manufacture, char *moduleNumber, char *deviceType, char *devSn, char *tansportType);
int updateConfigInfo(char *updateBuf, char *deviceType, char *manufacture, char *moduleNumber, char *devSn, char *configName, char *configValue);
int convertFromJsonToStr(char *json, char *str);

int parseDevOnlineBuf(char *jsonBuf, char *manufacture, char *moduleNumber, char *deviceType, char *devSn, char *transportType)
{
    cJSON *json, *jsonManufacture, *jsonModuleNumber, *jsonDeviceType, *jsonDeviceSn, *jsonTransport;
    int parseStatus = 0;

    /* parse jsonBuf */
    json = cJSON_Parse(jsonBuf);  

    if (!json)  
    {   
        printf("not json format\n ");
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());  
    }   

    else  
    {   
        /* parse item "manufacture" */ 
        jsonManufacture = cJSON_GetObjectItem(json, "manufacture");

        if (NULL == jsonManufacture)
        {
            printf("failed to get manufacture from json\n");
            return -1;
        }
        if (jsonManufacture->type == cJSON_String )   
        {  
            parseStatus ++;
            strncpy(manufacture, jsonManufacture->valuestring, MAX_MANUFACTURE_LEN - 1);
            printf("manufacture = %s\n", manufacture);  
        }  
        else
        {
            printf("manufacture value is not string\n");
        }

        /* parse item "moduleNumber" */ 
        jsonModuleNumber = cJSON_GetObjectItem(json, "moduleNumber");

        if (NULL == jsonModuleNumber)
        {
            printf("failed to get moduleNumber from json\n");
            return -1;
        }
        if (jsonModuleNumber->type == cJSON_String )   
        {  
            parseStatus ++;
            strncpy(moduleNumber, jsonModuleNumber->valuestring, MAX_MODULENUMBER_LEN - 1);
            printf("moduleNumber = %s\n", moduleNumber);  
        }  
        else
        {
            printf("moduleNumber value is not string\n");
        }

        /* parse item "deviceType" */ 
        jsonDeviceType = cJSON_GetObjectItem(json, "deviceType");

        if (NULL == jsonDeviceType)
        {
            printf("failed to get deviceType from json\n");
            return -1;
        }
        if (jsonDeviceType->type == cJSON_String )   
        {  
            parseStatus ++;
            strncpy(deviceType, jsonDeviceType->valuestring, MAX_DEVICETYPE_LEN - 1);
            printf("deviceType = %s\n", deviceType);  
        }  
        else
        {
            printf("deviceType value is not string\n");
        }
        /* parse item "serialNumber" */ 
        jsonDeviceSn = cJSON_GetObjectItem(json, "manufactureSN");

        if (NULL == jsonDeviceSn)
        {
            printf("failed to get devSn from json\n");
            return -1;
        }
        if (jsonDeviceSn->type == cJSON_String )   
        {  
            parseStatus ++;
            strncpy(devSn, jsonDeviceSn->valuestring, MAX_DEVICESN_LEN - 1);
            printf("devSn = %s\n", devSn);  
        }  
        else
        {
            printf("devSn value is not string\n");
        }

        /* parse item "transportType" */ 
        jsonTransport = cJSON_GetObjectItem(json, "transportType");

        if (NULL == jsonTransport)
        {
            printf("failed to get transportType from json\n");
            return -1;
        }
        if (jsonTransport->type == cJSON_String )   
        {  
            parseStatus ++;
            strncpy(transportType, jsonTransport->valuestring, MAX_TRANSPORT_LEN - 1);
            printf("transportType = %s\n", transportType);  
        }  
        else
        {
            printf("transport value is not string\n");
        }

        cJSON_Delete(json);  
        /* free */
    }

    if(parseStatus == 5)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int updateConfigInfo(char *updateBuf, char *deviceType, char *manufacture, char *moduleNumber, char *devSn, char *configName, char *configValue)
{
    char previousDevData[MAX_DEVDATA_LEN] = {0};
    cJSON *jsonUpdate, *jsonOrignal, *jsonManufacture, *jsonModuleNumber, *jsonDeviceType, *jsonDeviceSn;

    getDevDataByDevSn(devSn, previousDevData);
    /* parse jsonBuf */
    jsonUpdate = cJSON_Parse(updateBuf);  
    jsonOrignal = cJSON_Parse(previousDevData);  

    if((!jsonUpdate) ||(!jsonOrignal) ) 
    {   
        printf("not json format\n ");
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());  
        return -1;
    }   

    printf("updateBuf = %s\n", updateBuf);
    /* get node from DataConvert.xml */
    int i = 0;
    FILE *fp = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *node = NULL;
    mxml_node_t *deviceTypeNode = NULL;
    mxml_node_t *manufactureNode = NULL;
    mxml_node_t *configKeyNode = NULL;

    fp = fopen(DATA_CONVERT_FILE_PATH, "r");
    if(fp == NULL)
    {
        printf("open %s error!\n", DATA_CONVERT_FILE_PATH);
        return -1;
    }
    else
    {
        printf("open %s succeed!\n", DATA_CONVERT_FILE_PATH);
    }

    tree = mxmlLoadFile(NULL, fp,MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        printf("Load file error!\n");
        return -1;
    }
    else
    {
        printf("Load file succeed!\n");
    }

    node = mxmlFindElement(tree, tree, "node", NULL, NULL, MXML_DESCEND);
    if(node == NULL)
    {
        printf("can not find element node!\n");
        mxmlDelete(tree);
        return -1;
    }

    deviceTypeNode = mxmlFindElement(tree, tree, deviceType, NULL, NULL, MXML_DESCEND);
    if(deviceTypeNode == NULL)
    {
        printf("cannot find type node %s\n", deviceType);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        printf("find type node %s\n", deviceType);
    }

    manufactureNode = mxmlFindElement(deviceTypeNode, deviceTypeNode, manufacture, NULL, NULL, MXML_DESCEND);
    if(manufacture == NULL)
    {
        printf("cannot find manufacture node %s\n", manufacture);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        printf("find manufacture node %s\n", manufacture);
    }

    configKeyNode = mxmlFindElement(manufactureNode, manufactureNode, moduleNumber, NULL, NULL, MXML_DESCEND);
    if(configKeyNode == NULL)
    {
        printf("cannot find moduleNumber node %s\n", moduleNumber);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        printf("find moduleNumber node %s\n", moduleNumber);
    }
    mxml_node_t *node_config = NULL;
    mxml_node_t *node_detail = NULL;

    for(node_config = mxmlFindElement(configKeyNode, configKeyNode, "configKey", NULL, NULL, MXML_DESCEND); 
            node_config != NULL;
            node_config = mxmlFindElement(node_config, configKeyNode, "configKey", NULL, NULL, MXML_DESCEND))
    {
        printf("node_config = %p\n", node_config);
        printf("config txt: %s\n", mxmlElementGetAttr(node_config,"name"));

        if(strcmp(mxmlElementGetAttr(node_config,"name"), "devData") == 0)
        {
            printf("found devData\n");

            for(node_detail = mxmlFindElement(node_config, configKeyNode, "detail", NULL, NULL, MXML_DESCEND);
                    node_detail != NULL;
                    node_detail = mxmlFindElement(node_detail, node_config, "detail", NULL, NULL, MXML_DESCEND))
            {
                printf("node_detail = %p\n", node_detail);
                printf("detail txt: %s\n", mxmlElementGetAttr(node_detail,"name"));

                cJSON *json_keyNode = cJSON_GetObjectItem(jsonUpdate, mxmlElementGetAttr(node_detail,"name"));
                if(json_keyNode == NULL)
                {
                    printf("cannot find %s key in devData\n", mxmlElementGetAttr(node_detail,"name"));
                }
                else
                {
                    memset(configName, 0, MAX_CONFIGNAME_LEN);
                    memset(configValue, 0, MAX_CONFIGVALUE_LEN);
                    strncpy(configName, mxmlElementGetAttr(node_detail,"name"), MAX_CONFIGNAME_LEN - 1);
                    strncpy(configValue, json_keyNode->valuestring, MAX_CONFIGVALUE_LEN - 1);
                    printf("find %s key in devData\n", configName);

                    printf("configName = %s, configValue = %s\n", configName, configValue);
                    cJSON *json_keyNodeOrignal = cJSON_GetObjectItem(jsonOrignal, configName);

                    if(json_keyNodeOrignal != NULL)
                    {
                        cJSON_GetObjectItem(jsonOrignal, configName)->valuestring = strdup(configValue);
                        char *newDevData = cJSON_PrintUnformatted(jsonOrignal);
                        printf("newDevData = %s\n", newDevData);
                        setDevDataByDevSn(devSn, newDevData);
                        /* need to update to database */
                        free(newDevData);
#if 0
                        printf("verify again: \n");
                        char testDevData[MAX_DEVDATA_LEN] = {0};
                        getDevDataByDevSn(devSn, testDevData);
#endif

                    }
                }
            } 
        }
        else
        {
            printf("no need to handle this config\n");
        }
    }
    cJSON_Delete(jsonUpdate);  
    cJSON_Delete(jsonOrignal);  
    mxmlDelete(tree);
    /* free */

    return 0;
}

int convertFromJsonToStr(char *json, char *str)
{
    cJSON *jsonNode;
    jsonNode = cJSON_Parse(json);
    if(jsonNode != NULL)
    {
        char *s = cJSON_PrintUnformatted(jsonNode);
        if(s)
        {
            memset(str, 0, MAX_DEVDATA_LEN);
            strncpy(str, s, MAX_DEVDATA_LEN - 1);    
            free(s);
        }
        cJSON_Delete(jsonNode);
    }
    return 0;

}




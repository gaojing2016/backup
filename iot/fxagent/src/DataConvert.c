#include <mxml.h>
#include "share.h"
#include "parser.h"
#include <sqlite3.h>

#define KEY_NAME_MAX_LEN 30
#define KEY_NAME_PATH_MAX_LEN 60

int getAlljoynKeyName(char *deviceType, char *manufacture, char *moduleNumber, struct ipk_info *platformKeyName, int keyNameNumber);
int getConfigKeyFromXml(char *deviceType, char *manufacture, char *moduleNumber, char *devDataName, struct ipk_info *configKeyHead);
void getObjInfoByDevId(char *deviceId, char *interfaceName, char *objectPath, char *onlineStatus);
void getDevInfoByDevId(char *deviceId, char *deviceType, char *manufacture, char *moduleNumber);
void getDevInfoByObj(char *interfaceName, char *objectPath, char *deviceId, char *deviceType);
void getDevDataByObj(char *interfaceName, char *devData); 
void getDevDataByDevId(char *deviceId, char *devData); 
int setDevIdByDevSn(char *deviceId, char *deviceSn);


/* deviceId ->>>> interfaceName, objectPath, deviceType, onlineStatus */
void getObjInfoByDevId(char *deviceId, char *interfaceName, char *objectPath, char *onlineStatus)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0; 

        snprintf(sqlCmd, sizeof(sqlCmd), "select interface, objpath, STATUS from deviceinfo WHERE deviceid = '%s'", deviceId );
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        cloudc_debug("%s[%d]: query result: nrow = %d, ncolumn = %d", __func__, __LINE__, nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            cloudc_debug("%s[%d]: query %i records.", __func__, __LINE__, nrow);
            index = ncolumn;

            for(i = 0; i < nrow; i++)
            {
                cloudc_debug("%s[%d]: content in row [%d], index = %d:", __func__, __LINE__, i, index);
                for(j = 0; j < ncolumn; j++)
                { 
                    cloudc_debug("%s[%d]: j = %d %s\n", __func__, __LINE__, j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(interfaceName, 0, MAX_INTERFACE_LEN);
                            strncpy(interfaceName, dbresult[index], MAX_INTERFACE_LEN - 1);
                            cloudc_debug("%s[%d]: interfaceName = %s", __func__, __LINE__, interfaceName);
                            break;
                        case 1:
                            memset(objectPath, 0, MAX_OBJECTPATH_LEN);
                            strncpy(objectPath, dbresult[index], MAX_OBJECTPATH_LEN - 1);
                            cloudc_debug("%s[%d]: objectPath = %s", __func__, __LINE__, objectPath);
                            break;
                        case 2:
                            memset(onlineStatus, 0, MAX_ONLINESTATUS_LEN);
                            strncpy(onlineStatus, dbresult[index], MAX_ONLINESTATUS_LEN - 1);
                            cloudc_debug("%s[%d]: onlineStatus = %s", __func__, __LINE__, onlineStatus);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }
        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
            fprintf(stderr,"Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);

        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        fprintf(stderr,"failed to open this db file: %s\n", sqlite3_errmsg(db));
        sqlite3_free(errmsg);
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

/* deviceId ->>>> manufacture, moduleNumber */
void getDevInfoByDevId(char *deviceId, char *deviceType, char *manufacture, char *moduleNumber)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0; 

        snprintf(sqlCmd, sizeof(sqlCmd), "select devicetype, manufacture, modulenumber from deviceinfo WHERE deviceid = '%s'", deviceId );
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        cloudc_debug("%s[%d]: query result: nrow = %d, ncolumn = %d", __func__, __LINE__, nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            cloudc_debug("%s[%d]: query %i records.", __func__, __LINE__, nrow);
            index = ncolumn;

            for(i = 0; i < nrow; i++)
            {
                cloudc_debug("%s[%d]: content in row [%d], index = %d:", __func__, __LINE__, i, index);
                for(j = 0; j < ncolumn; j++)
                { 
                    cloudc_debug("%s[%d]: j = %d %s\n", __func__, __LINE__, j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(deviceType, 0, MAX_DEVICETYPE_LEN);
                            strncpy(deviceType, dbresult[index], MAX_DEVICETYPE_LEN - 1);
                            cloudc_debug("%s[%d]: deviceType = %s", __func__, __LINE__, deviceType);
                            break;
                        case 1:
                            memset(manufacture, 0, MAX_MANUFACTURE_LEN);
                            strncpy(manufacture, dbresult[index], MAX_MANUFACTURE_LEN - 1);
                            cloudc_debug("%s[%d]: manufacture = %s", __func__, __LINE__, manufacture);
                        case 2:
                            memset(moduleNumber, 0, MAX_MODULENUMBER_LEN);
                            strncpy(moduleNumber, dbresult[index], MAX_MODULENUMBER_LEN - 1);
                            cloudc_debug("%s[%d]: moduleNumber = %s", __func__, __LINE__, moduleNumber);
                        default:
                            break;
                    }
                    index++;
                }
            }
        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
            fprintf(stderr,"Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        fprintf(stderr,"failed to open this db file: %s\n", sqlite3_errmsg(db));
        sqlite3_free(errmsg);
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;

}

/* interfaceName, objectPath ->>>> deviceId, deviceType */
void getDevInfoByObj(char *interfaceName, char *objectPath, char *deviceId, char *deviceType)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0; 

        snprintf(sqlCmd, sizeof(sqlCmd), "select deviceid, deviceType from deviceinfo WHERE interface = '%s'", interfaceName);
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        cloudc_debug("%s[%d]: query result: nrow = %d, ncolumn = %d", __func__, __LINE__, nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            cloudc_debug("%s[%d]: query %i records.", __func__, __LINE__, nrow);
            index = ncolumn;

            for(i = 0; i < nrow; i++)
            {
                cloudc_debug("%s[%d]: content in row [%d], index = %d:", __func__, __LINE__, i, index);
                for(j = 0; j < ncolumn; j++)
                { 
                    cloudc_debug("%s[%d]: j = %d %s\n", __func__, __LINE__, j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(deviceId, 0, MAX_DEVICEID_LEN);
                            strncpy(deviceId, dbresult[index], MAX_DEVICEID_LEN - 1);
                            break;
                        case 1:
                            memset(deviceType, 0, MAX_DEVICETYPE_LEN);
                            strncpy(deviceType, dbresult[index], MAX_DEVICETYPE_LEN - 1);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }
        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
            fprintf(stderr,"Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        fprintf(stderr,"failed to open this db file: %s\n", sqlite3_errmsg(db));
        sqlite3_free(errmsg);
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;

}

/* interfaceName ->>>> devData */
void getDevDataByObj(char *interfaceName, char *devData)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0; 

        snprintf(sqlCmd, sizeof(sqlCmd), "select devData from deviceinfo WHERE interface = '%s'", interfaceName);
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        cloudc_debug("%s[%d]: query result: nrow = %d, ncolumn = %d", __func__, __LINE__, nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            cloudc_debug("%s[%d]: query %i records.", __func__, __LINE__, nrow);
            index = ncolumn;

            for(i = 0; i < nrow; i++)
            {
                cloudc_debug("%s[%d]: content in row [%d], index = %d:", __func__, __LINE__, i, index);
                for(j = 0; j < ncolumn; j++)
                { 
                    cloudc_debug("%s[%d]: j = %d %s\n", __func__, __LINE__, j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(devData, 0, MAX_DEVDATA_LEN);
                            strncpy(devData, dbresult[index], MAX_DEVDATA_LEN - 1);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }
        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
            fprintf(stderr,"Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        fprintf(stderr,"failed to open this db file: %s\n", sqlite3_errmsg(db));
        sqlite3_free(errmsg);
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

void getDevDataByDevId(char *deviceId, char *devData)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0; 

        snprintf(sqlCmd, sizeof(sqlCmd), "select devData from deviceinfo WHERE deviceid = '%s'", deviceId);
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        cloudc_debug("%s[%d]: query result: nrow = %d, ncolumn = %d", __func__, __LINE__, nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            cloudc_debug("%s[%d]: query %i records.", __func__, __LINE__, nrow);
            index = ncolumn;

            for(i = 0; i < nrow; i++)
            {
                cloudc_debug("%s[%d]: content in row [%d], index = %d:", __func__, __LINE__, i, index);
                for(j = 0; j < ncolumn; j++)
                { 
                    cloudc_debug("%s[%d]: j = %d %s\n", __func__, __LINE__, j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(devData, 0, MAX_DEVDATA_LEN);
                            strncpy(devData, dbresult[index], MAX_DEVDATA_LEN - 1);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }
        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
            fprintf(stderr,"Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        fprintf(stderr,"failed to open this db file: %s\n", sqlite3_errmsg(db));
        sqlite3_free(errmsg);
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

int setDevIdByDevSn(char *deviceId, char *deviceSn)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * errmsg = 0;
    int result = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {   
        cloudc_debug("%s[%d]: You have opened a sqlite3 database named device.db successfully!", __func__, __LINE__);
        /* query the data you want */
        char *errmsg = 0;
        char sqlCmd[128] = {0};

        snprintf(sqlCmd, sizeof(sqlCmd), "UPDATE deviceinfo set deviceid = '%s' WHERE devicesn = '%s'", deviceId, deviceSn );
        cloudc_debug("%s[%d]: sql cmd = %s", __func__, __LINE__, sqlCmd);

        ret = sqlite3_exec(db, sqlCmd, NULL, NULL, &errmsg);
        if (ret != SQLITE_OK) { 
            fprintf(stderr,"update table deviceinfo Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
            ret = -1; 
        }
        else
        {
            cloudc_debug("%s[%d]: update deviceId value succeed", __func__, __LINE__);
            ret = 0;
        }
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return ret;
}


int getAlljoynKeyName(char *deviceType, char *manufacture, char *moduleNumber, struct ipk_info *platformKeyName, int keyNameNumber)
{
    int i = 0;
    char platformKeyNameNodePath[KEY_NAME_PATH_MAX_LEN] = {0};
    FILE *fp = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *platformkeyNameNode = NULL;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: deviceType = %s, manufacture = %s, platformKeyNameHead = %p", __func__, __LINE__, deviceType, manufacture, platformKeyName);

    fp = fopen(DATA_CONVERT_FILE_PATH, "r");
    if(fp == NULL)
    {
        cloudc_error("%s[%d]: open %s error!", __func__, __LINE__, DATA_CONVERT_FILE_PATH);
        return -1;
    }
    else
    {
        cloudc_debug("%s[%d]: open %s succeed!", __func__, __LINE__, DATA_CONVERT_FILE_PATH);
    }

    tree = mxmlLoadFile(NULL, fp,MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        cloudc_error("%s[%d]: Load file error!", __func__, __LINE__);
        return -1;
    }
    else
    {
        cloudc_debug("%s[%d]: Load file success!", __func__, __LINE__);
    }

    if(platformKeyName != NULL)
    {
        for(i = 0; i < keyNameNumber; i ++)
        {
            snprintf(platformKeyNameNodePath, KEY_NAME_PATH_MAX_LEN, "node/%s/%s/%s/%s", deviceType, manufacture, moduleNumber, platformKeyName->op_ipk_name);
            cloudc_debug("%s[%d]: platformKeyNamePath = %s", __func__, __LINE__, platformKeyNameNodePath);

            platformkeyNameNode = mxmlFindPath(tree, platformKeyNameNodePath);
            if(platformkeyNameNode == NULL)
            {
                cloudc_error("%s[%d]: cannot find element %s\n", __func__, __LINE__, platformKeyName->op_ipk_name);
                return -1;
            }
            else
            {
                cloudc_debug("%s[%d]: find element %s\n", __func__, __LINE__, platformKeyName->op_ipk_name);
            }

            strncpy(platformKeyName->node_name_convert, mxmlElementGetAttr(platformkeyNameNode,"alljoyn"), MAX_IPK_NAME_LEN - 1);

            memset(platformKeyNameNodePath, 0, sizeof(platformKeyNameNodePath));
            platformKeyName = platformKeyName->next;
        }
    }
    mxmlDelete(tree);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int getConfigKeyFromXml(char *deviceType, char *manufacture, char *moduleNumber, char *devDataName, struct ipk_info *configKeyHead)
{
    int i = 0;
    FILE *fp = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *node = NULL;
    mxml_node_t *deviceTypeNode = NULL;
    mxml_node_t *manufactureNode = NULL;
    mxml_node_t *configKeyNode = NULL;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    fp = fopen(DATA_CONVERT_FILE_PATH, "r");
    if(fp == NULL)
    {
        cloudc_error("%s[%d]: open %s error!", __func__, __LINE__, DATA_CONVERT_FILE_PATH);
        return -1;
    }
    else
    {
        cloudc_debug("%s[%d]: open %s succeed!", __func__, __LINE__, DATA_CONVERT_FILE_PATH);
    }

    tree = mxmlLoadFile(NULL, fp,MXML_TEXT_CALLBACK);
    fclose(fp);

    if(tree == NULL)
    {
        cloudc_error("%s[%d]: Load file error!", __func__, __LINE__);
        return -1;
    }
    else
    {
        cloudc_debug("%s[%d]: Load file success!", __func__, __LINE__);
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
        cloudc_error("%s[%d]: cannot find type node %s\n", __func__, __LINE__, deviceType);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        cloudc_error("%s[%d]: find type node %s\n", __func__, __LINE__, deviceType);
    }

    manufactureNode = mxmlFindElement(deviceTypeNode, deviceTypeNode, manufacture, NULL, NULL, MXML_DESCEND);
    if(manufacture == NULL)
    {
        cloudc_error("%s[%d]: cannot find manufacture node %s\n", __func__, __LINE__, manufacture);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        cloudc_error("%s[%d]: find manufacture node %s\n", __func__, __LINE__, manufacture);
    }

    configKeyNode = mxmlFindElement(manufactureNode, manufactureNode, moduleNumber, NULL, NULL, MXML_DESCEND);
    if(configKeyNode == NULL)
    {
        cloudc_error("%s[%d]: cannot find moduleNumber node %s\n", __func__, __LINE__, moduleNumber);
        mxmlDelete(tree);
        return -1;
    }
    else
    {
        cloudc_error("%s[%d]: find moduleNumber node %s\n", __func__, __LINE__, moduleNumber);
    }

    mxml_node_t *node_config = NULL;
    mxml_node_t *node_detail = NULL;

    for(node_config = mxmlFindElement(configKeyNode, configKeyNode, "configKey", NULL, NULL, MXML_DESCEND); node_config != NULL;
            node_config = mxmlFindElement(node_config, configKeyNode, "configKey", NULL, NULL, MXML_DESCEND)) 
    {
        printf("node_config = %p\n", node_config);
        printf("config txt: %s\n", mxmlElementGetAttr(node_config,"name"));

        if(strcmp(mxmlElementGetAttr(node_config,"name"), "devData") == 0)
        {
            strcpy(devDataName, "devData");
            printf("found devData: %s\n", devDataName);
        }
        else
        {
            cloudc_get_ipk_name(configKeyHead, mxmlElementGetAttr(node_config,"name"), NULL);
        }

        for(node_detail = mxmlFindElement(node_config, configKeyNode, "detail", NULL, NULL, MXML_DESCEND);
                node_detail != NULL;
                node_detail = mxmlFindElement(node_detail, node_config, "detail", NULL, NULL, MXML_DESCEND)) 
        {
            printf("node_detail = %p\n", node_detail);
            cloudc_debug("%s[%d]: detail txt: %s", __func__, __LINE__, mxmlElementGetAttr(node_detail,"name"));
        }   
    }
    printf("hello\n");

    mxmlDelete(tree);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

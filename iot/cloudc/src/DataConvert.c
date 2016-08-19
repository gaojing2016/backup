#include <mxml.h>
#include "share.h"
#include "parser.h"
#include <sqlite3.h>

#define KEY_NAME_MAX_LEN 30
#define KEY_NAME_PATH_MAX_LEN 60
#define DATA_CONVERT_FILE_PATH "/etc/config/DataConvert.xml"
#define DEVICE_DATABASE_PATH "/usr/bin/device.db"

int getAlljoynKeyName(char *deviceType, char *platform, struct ipk_info *platformKeyName, int keyNameNumber);
void getObjInfoByDevId(char *deviceId, char *interfaceName, char *objectPath, char *deviceType, char *onlineStatus);
void getDevInfoByObj(char *interfaceName, char *objectPath, char *deviceId, char *deviceType);

/* deviceId ->>>> interfaceName, objectPath, deviceType, onlineStatus */
void getObjInfoByDevId(char *deviceId, char *interfaceName, char *objectPath, char *deviceType, char *onlineStatus)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * pErrMsg = 0;
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
        
        snprintf(sqlCmd, sizeof(sqlCmd), "select interface, objpath, deviceType, STATUS from deviceinfo WHERE deviceid = '%s'", deviceId );
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
                            memset(interfaceName, 0, sizeof(interfaceName));
                            strcpy(interfaceName, dbresult[index]);
                            break;
                        case 1:
                            memset(objectPath, 0, sizeof(objectPath));
                            strcpy(objectPath, dbresult[index]);
                            break;
                        case 2:
                            memset(deviceType, 0, sizeof(deviceType));
                            strcpy(deviceType, dbresult[index]);
                            break;
                        case 3:
                            memset(onlineStatus, 0, sizeof(onlineStatus));
                            strcpy(onlineStatus, dbresult[index]);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }

            //cloudc_debug("%s[%d]:\n interfaceName = %s, \nobjectPath = %s, \ndeviceType = %s", __func__, __LINE__, interfaceName, objectPath, deviceType);

        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        cloudc_error(stderr, "failed to open this db file: %s", sqlite3_errmsg(db));
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
    char * pErrMsg = 0;
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
        //snprintf(sqlCmd, sizeof(sqlCmd), "select deviceid, deviceType from deviceinfo WHERE objpath = '%s'", objectPath);
        //snprintf(sqlCmd, sizeof(sqlCmd), "select deviceid, deviceType from deviceinfo WHERE interface = '%s' AND objpath = '%s'", interfaceName, objectPath);
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
                            memset(deviceId, 0, sizeof(deviceId));
                            strcpy(deviceId, dbresult[index]);
                            break;
                        case 1:
                            memset(deviceType, 0, sizeof(deviceType));
                            strcpy(deviceType, dbresult[index]);
                            break;
                        default:
                            break;
                    }
                    index++;
                }
            }

            //cloudc_debug("%s[%d]:\ndeviceId = %s, \ndeviceType = %s", __func__, __LINE__, deviceId, deviceType);

        }
        else
        {
            cloudc_debug("%s[%d]: query failed", __func__, __LINE__);
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        cloudc_error("%s[%d]: %s", __func__, __LINE__, DEVICE_DATABASE_PATH);
        cloudc_error(stderr, "failed to open this db file: %s", sqlite3_errmsg(db));
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;

}

int getAlljoynKeyName(char *deviceType, char *platform, struct ipk_info *platformKeyName, int keyNameNumber)
{
    int i = 0;
    char *seperate = ";";
    char platformKeyNameNodePath[KEY_NAME_PATH_MAX_LEN] = {0};
    FILE *fp = NULL;
    mxml_node_t *tree = NULL;
    mxml_node_t *platformkeyNameNode = NULL;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: deviceType = %s, platform = %s, platformKeyNameHead = %p", __func__, __LINE__, deviceType, platform, platformKeyName);

    fp = fopen(DATA_CONVERT_FILE_PATH, "r");
    cloudc_debug("%s[%d]: here", __func__, __LINE__);

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
            snprintf(platformKeyNameNodePath, KEY_NAME_PATH_MAX_LEN, "node/%s/%s/%s", deviceType, platform, platformKeyName->op_ipk_name);
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


#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "share.h"
#define DEVICE_DATABASE_PATH "/usr/bin/device.db"

char *getOnlineStatusByDevSn(char *manufacture, char *deviceSn, char *onlineStatus, char *deviceType);
void getDevDataByDevSn(char *deviceSn, char *devData);
int setDevDataByDevSn(char *deviceSn, char *devData);

/* manufacture, devSn ->>>> onlineStatus */
char *getOnlineStatusByDevSn(char *manufacture, char *deviceSn, char *onlineStatus, char *deviceType)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * pErrMsg = 0;
    int result = 0;

    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {   
        printf("You have opened a sqlite3 database named device.db successfully!\n");
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0;  

        snprintf(sqlCmd, sizeof(sqlCmd), "select STATUS from deviceinfo WHERE manufacture = '%s' AND deviceSn = '%s'", manufacture, deviceSn);
        printf("sql cmd = %s\n", sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        printf("query result: nrow = %d, ncolumn = %d\n", nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            printf("query %i records.\n", nrow);
            index = ncolumn;
            if(0 == nrow)
            {
                memset(onlineStatus, 0, MAX_ONLINESTATUS_LEN);
            }
            else
            {
                for(i = 0; i < nrow; i++)
                {
                    printf("content in row [%d], index = %d: \n", i, index);
                    for(j = 0; j < ncolumn; j++)
                    {
                        printf("j = %d %s\n", j, dbresult[index]);
                        switch(j)
                        {
                            case 0:
                                memset(onlineStatus, 0, MAX_ONLINESTATUS_LEN);
                                strncpy(onlineStatus, dbresult[index], MAX_ONLINESTATUS_LEN - 1); 
                                break;
                            default:
                                break;
                        }
                        index++;
                    }
                }
            }
        }
        else
        {
            printf("query failed\n");
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        fprintf(stderr,"insert value Error: %s\n", sqlite3_errmsg(db));
    }

    /* close */
    sqlite3_close(db);
    db = 0;

    return onlineStatus;
}

void getDevDataByDevSn(char *deviceSn, char *devData)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * pErrMsg = 0;
    int result = 0;
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    printf("Enter getDevDataByDevSn\n");

    if(ret == SQLITE_OK)
    {
        printf("You have opened a sqlite3 database named device.db successfully!\n");
        /* query the data you want */
        char **dbresult;
        char *errmsg = 0;
        char sqlCmd[128] = {0};
        int i = 0;
        int j = 0;
        int nrow = 0;
        int ncolumn = 0;
        int index = 0;

        snprintf(sqlCmd, sizeof(sqlCmd), "select devData from deviceinfo WHERE devicesn = '%s'", deviceSn);
        printf("sql cmd = %s\n", sqlCmd);
        ret = sqlite3_get_table(db, sqlCmd, &dbresult, &nrow, &ncolumn, &errmsg);
        printf("query result: nrow = %d, ncolumn = %d\n", nrow, ncolumn);
        if(ret == SQLITE_OK)
        {
            printf("query %i records.\n", nrow);
            index = ncolumn;
            for(i = 0; i < nrow; i++)
            {
                printf("content in row [%d], index = %d:\n", i, index);
                for(j = 0; j < ncolumn; j++)
                {
                    printf("j = %d %s\n", j, dbresult[index]);
                    switch(j)
                    {
                        case 0:
                            memset(devData, 0, sizeof(MAX_DEVDATA_LEN));
                            strncpy(devData, dbresult[index], MAX_DEVDATA_LEN - 1);
                            printf("devData = %s\n", devData);
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
            printf("query failed\n");
        }
        sqlite3_free_table(dbresult);
    }
    else
    {
        fprintf(stderr, "failed to open this db file: %s\n", sqlite3_errmsg(db));
    }
    /* close */
    sqlite3_close(db);
    db = 0;

    printf("Exit getDevDataByDevSn\n");
    return;
}


int setDevDataByDevSn(char *deviceSn, char *devData)
{
    /* connect to database */
    sqlite3 *db = 0;
    char * pErrMsg = 0;
    int result = 0;

    printf("Enter setDevDataByDevSn\n");
    int ret = sqlite3_open(DEVICE_DATABASE_PATH, &db);

    if(ret == SQLITE_OK)
    {
        printf("You have opened a sqlite3 database named device.db successfully!\n");
        /* query the data you want */
        char *errmsg = 0;
        char sqlCmd[128+MAX_DEVDATA_LEN] = {0};

        snprintf(sqlCmd, sizeof(sqlCmd), "UPDATE deviceinfo set devData = '%s' WHERE devicesn = '%s'", devData, deviceSn );
        printf("sql cmd = %s\n", sqlCmd);
        ret = sqlite3_exec(db, sqlCmd, NULL, NULL, &errmsg);
        if (ret != SQLITE_OK) {
            fprintf(stderr,"update table deviceinfo Error: %s\n", sqlite3_errmsg(db));
            sqlite3_free(errmsg);
            return -1;
        }
        else
        {
            printf("update devData value succeed\n");
        }
    }

    /* close */
    sqlite3_close(db);
    db = 0;
    printf("Exit setDevDataByDevSn\n");
    return 0;
}


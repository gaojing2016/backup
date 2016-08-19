#include <mxml.h>
#include <sqlite3.h>

#define KEY_NAME_MAX_LEN 30
#define KEY_NAME_PATH_MAX_LEN 60

int getConfigKeyFromXml(char *deviceType, char *manufacture, char *moduleNumber, char *devDataName, char *configName, char *configValue);

int getConfigKeyFromXml(char *deviceType, char *manufacture, char *moduleNumber, char *devDataName, char *configName, char *configValue)
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
            //strncpy(configName, mxmlElementGetAttr(node_config,"name"), sizeof(configName) - 1);
            continue;
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

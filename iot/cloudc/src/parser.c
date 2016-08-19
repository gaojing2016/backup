#include "share.h"
#include "parser.h"
#include "cJSON.h"

int cloudc_parse_receive_info(char *recvbuf);
int cloudc_get_type(char *op_type);
int cloudc_get_serial_num(int serial_num);
int cloudc_parse_http_header(char *recvbuf);
char *cloudc_get_http_body(char *recvbuf);
int cloudc_parse_http_body(char *json_buf);
int cloudc_parse_alljoyn_notification(const char *json_buf);
void cloudc_get_ipk_name(struct ipk_info *list_head, char *ipk_name, char *node_value);
void cloudc_print_ipk_name_list(struct ipk_info *list_head);
void cloudc_destroy_ipk_name(struct ipk_info *list_head);
char *define_data_by_device_type(char *device_type);

http_value recvdata;
int rsp_status = 0;
char keyName[3][30] = {"power_switch", "status", "color_rgb"};

int cloudc_parse_receive_info(char *recvbuf)
{
    char *http_body = NULL;
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    if (0 == cloudc_parse_http_header(recvbuf))
    {
        http_body = cloudc_get_http_body(recvbuf);

        if (NULL != http_body)
        {
            cloudc_parse_http_body(http_body);
        }
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_parse_http_header(char *recvbuf)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    /* need to parse http value firstly
     * such as return code
     * ...
     * if return code is ok, then return 0 and go on parse body value
     * */

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

char *cloudc_get_http_body(char *recvbuf)
{
    char *http_body = NULL;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    http_body = strstr(recvbuf, "{");

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return http_body;
}

int cloudc_parse_http_body(char *json_buf)
{
    struct ipk_info *head = NULL;
    head = (struct ipk_info *)malloc(sizeof(struct ipk_info));

    if (NULL == head)
    {
        cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    memset(head, 0, sizeof(struct ipk_info));

    int recv_status_count = 0;
    int i = 0;
    eOpType flag;

    char *array_item = NULL;
    char *array_item_value = NULL;
    char array_item_tmp[MAX_IPK_NAME_LEN] = {0};

    cJSON *json, *json_type, *json_serial, *json_status, *json_package, *json_update, *json_url, *json_user_id, *json_device_id, *json_device_type, *json_config;
    cJSON *p_array_item = NULL;

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    cloudc_debug("%s[%d]: json_buf = %s", __func__, __LINE__, json_buf);

    /* parse json_buf */
    json = cJSON_Parse(json_buf);  

    if (!json)  
    {   
        cloudc_debug("%s[%d]: not json format ", __func__, __LINE__);
        cloudc_error("%s[%d]: Error before: [%s]\n", __func__, __LINE__, cJSON_GetErrorPtr());  
    }   

    else  
    {   
        memset(&recvdata,0,sizeof(recvdata)); 
        /* parse item "type" */ 
        json_type = cJSON_GetObjectItem(json, "type");  

        if (NULL == json_type)
        {
            cloudc_debug("%s[%d]: failed to get type from json", __func__, __LINE__);
        }
        else if ((json_type->type == cJSON_String) && (NULL != json_type->valuestring))   
        {  
            cloudc_debug("%s[%d]: type value = %s", __func__, __LINE__, json_type->valuestring);  

            recvdata.rpc_flag = cloudc_get_type(json_type->valuestring);
            recv_status_count++;
            cloudc_debug("%s[%d]: rpc_flag = %d", __func__, __LINE__, recvdata.rpc_flag);  

            json_serial = cJSON_GetObjectItem(json, "serial");

            if (NULL == json_serial)
            {
                cloudc_debug("%s[%d]: failed to get serial from json", __func__, __LINE__);
            }
            else if (json_serial->type == cJSON_Number )   
            {  
                recvdata.serial_num = cloudc_get_serial_num(json_serial->valueint);
                recv_status_count++;
                cloudc_debug("%s[%d]: serial number = %d", __func__, __LINE__, json_serial->valueint);  
            }  
            else
            {
                cloudc_error("%s[%d]: serial value is not int", __func__, __LINE__);
            }

            switch(recvdata.rpc_flag)
            {
                case eRegister: /* need three key words: type, serial, status */

                    json_status = cJSON_GetObjectItem(json, "status");  

                    if (NULL == json_status)
                    {
                        cloudc_debug("%s[%d]: failed to get status from json", __func__, __LINE__);
                    }
                    else if (json_status->type == cJSON_Number )   
                    {  
                        recvdata.register_status = json_status->valueint;
                        recv_status_count++;
                        cloudc_debug("%s[%d]: register_status = %d", __func__, __LINE__, json_status->valueint);  
                    } 
                    else
                    {
                        cloudc_error("%s[%d]: status value is not int");
                    }


                    if (3 == recv_status_count)
                    {
                        rsp_status = 1;
                        /* ap_register_flag = recvdata.register_status; */
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_debug("%s[%d]:  rsp_status = %d, correct parameter, will continue to handle it", __func__, __LINE__, rsp_status);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eQueryOperation: /* need two key words: type, serial */

                    if (2 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eOpkgOperation: /* need four key words: type, serial, update, url */

                    json_update = cJSON_GetObjectItem(json, "update");  

                    if (NULL == json_update)
                    {
                        cloudc_debug("%s[%d]: failed to get update from json", __func__, __LINE__);
                    }
                    else if (json_update->type == cJSON_Number )   
                    {  
                        recvdata.opkg_update_flag = json_update->valueint;
                        recv_status_count++;
                        cloudc_debug("%s[%d]: opkg_update_flag = %d", __func__, __LINE__, json_update->valueint);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: update value is not int");
                    }


                    json_url = cJSON_GetObjectItem(json, "url");

                    if (NULL == json_url)
                    {
                        cloudc_debug("%s[%d]: failed to get url from json", __func__, __LINE__);
                    }
                    else if (json_url->type == cJSON_String)
                    {
                        strncpy(recvdata.opkg_update_url, json_url->valuestring, sizeof(recvdata.opkg_update_url) - 1);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: opkg_update_url = %s", __func__, __LINE__, json_url->valuestring);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: url value is not string");
                    }


                    if (4 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eServiceOperation: /* need three key words: type, serial, package */

                    json_package = cJSON_GetObjectItem(json, "package");  

                    if (NULL == json_package)
                    {
                        cloudc_debug("%s[%d]: failed to get package from json", __func__, __LINE__);
                    }
                    else if (json_package->type == cJSON_Array)
                    {
                        recvdata.real_ipk_num = cJSON_GetArraySize(json_package);

                        if (0 < recvdata.real_ipk_num)
                        {
                            recv_status_count++;

                            while (i < recvdata.real_ipk_num)
                            {
                                p_array_item = cJSON_GetArrayItem(json_package, i);
                                array_item  = cJSON_Print(p_array_item); 
                                array_item[strlen(array_item) - 1] = '\0';
                                strncpy(array_item_tmp, &array_item[1], MAX_IPK_NAME_LEN - 1);
                                cloudc_get_ipk_name(head, array_item_tmp, NULL);
                                i++;
                            }

                            recvdata.ipk_name_head = head;
                        }
                    }
                    else
                    {
                        cloudc_error("%s[%d]: package value is not array");
                    }

                    if (3 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eIpkOperation: /* need three key words: type, serial, package */

                    json_package = cJSON_GetObjectItem(json, "package");  

                    if (NULL == json_package)
                    {
                        cloudc_debug("%s[%d]: failed to get package from json", __func__, __LINE__);
                    }
                    else if (json_package->type == cJSON_Array)
                    {
                        recvdata.real_ipk_num = cJSON_GetArraySize(json_package);

                        if (0 < recvdata.real_ipk_num)
                        {
                            recv_status_count++;

                            while (i < recvdata.real_ipk_num)
                            {
                                p_array_item = cJSON_GetArrayItem(json_package, i);
                                array_item  = cJSON_Print(p_array_item); 
                                array_item[strlen(array_item) - 1] = '\0';
                                strncpy(array_item_tmp, &array_item[1], MAX_IPK_NAME_LEN - 1);
                                cloudc_get_ipk_name(head, array_item_tmp, NULL);
                                i++;
                            }

                            recvdata.ipk_name_head = head;
                        }
                    }
                    else
                    {
                        cloudc_error("%s[%d]: package value is not array");
                    }


                    if (3 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eAlljoynGetOperation:

                    cloudc_debug("%s[%d]: This is eAlljoynGetOperation flag", __func__, __LINE__);

                    json_user_id = cJSON_GetObjectItem(json, "user_id");

                    if (NULL == json_user_id)
                    {
                        cloudc_debug("%s[%d]: failed to get user_id from json", __func__, __LINE__);
                    }
                    else if (json_user_id->type == cJSON_String)
                    {
                        strncpy(recvdata.user_id, json_user_id->valuestring, MAX_USER_ID_LEN);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: user_id = %s", __func__, __LINE__, recvdata.user_id);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: user_id value is not string", __func__, __LINE__);
                    }

                    json_device_id = cJSON_GetObjectItem(json, "device_id");

                    if (NULL == json_device_id)
                    {
                        cloudc_debug("%s[%d]: failed to get device_id from json", __func__, __LINE__);
                    }
                    else if (json_device_id->type == cJSON_String)
                    {
                        strncpy(recvdata.device_id, json_device_id->valuestring, MAX_DEVICE_ID_LEN);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: device_id = %s", __func__, __LINE__, recvdata.device_id);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: device_id value is not string", __func__, __LINE__);
                    }

                    json_device_type = cJSON_GetObjectItem(json, "device_type");

                    if (NULL == json_device_type)
                    {
                        cloudc_debug("%s[%d]: failed to get device_type from json", __func__, __LINE__);
                    }
                    else if (json_device_type->type == cJSON_String)
                    {
                        strncpy(recvdata.device_type, json_device_type->valuestring, MAX_DEVICE_TYPE_LEN - 1);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: device_type = %s", __func__, __LINE__, recvdata.device_type);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: device_type value is not string", __func__, __LINE__);
                    }

                    json_config = cJSON_GetObjectItem(json, "config");

                    if (NULL == json_config)
                    {
                        cloudc_debug("%s[%d]: failed to get config from json", __func__, __LINE__);
                    }
                    else if (json_config->type == cJSON_Array)
                    {
                        recvdata.real_ipk_num = cJSON_GetArraySize(json_config);
                        cloudc_debug("%s[%d]: recvdata.real_ipk_num = %d\n", __func__, __LINE__, recvdata.real_ipk_num);

                        if (0 < recvdata.real_ipk_num)
                        {
                            recv_status_count++;

                            while (i < recvdata.real_ipk_num)
                            {
                                p_array_item = cJSON_GetArrayItem(json_config, i);
                                array_item  = cJSON_Print(p_array_item); 
                                array_item[strlen(array_item) - 1] = '\0';
                                strncpy(array_item_tmp, &array_item[1], MAX_IPK_NAME_LEN - 1);
                                cloudc_get_ipk_name(head, array_item_tmp, NULL);
                                i++;
                            }

                            recvdata.ipk_name_head = head;
                        }
                    }
                    else
                    {
                        cloudc_error("%s[%d]: config value is not array", __func__, __LINE__);
                    }

                    if (6 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_alljoyn_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, recvdata.user_id, recvdata.device_id, recvdata.device_type, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_alljoyn_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, recvdata.user_id, recvdata.device_id, recvdata.device_type, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;

                case eAlljoynSetOperation:

                    cloudc_debug("%s[%d]: This is eAlljoynSetOperation flag", __func__, __LINE__);

                    json_user_id = cJSON_GetObjectItem(json, "user_id");

                    if (NULL == json_user_id)
                    {
                        cloudc_debug("%s[%d]: failed to get user_id from json", __func__, __LINE__);
                    }
                    else if (json_user_id->type == cJSON_String)
                    {
                        strncpy(recvdata.user_id, json_user_id->valuestring, MAX_USER_ID_LEN);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: user_id = %s", __func__, __LINE__, recvdata.user_id);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: user_id value is not string", __func__, __LINE__);
                    }


                    json_device_id = cJSON_GetObjectItem(json, "device_id");

                    if (NULL == json_device_id)
                    {
                        cloudc_debug("%s[%d]: failed to get device_id from json", __func__, __LINE__);
                    }
                    else if (json_device_id->type == cJSON_String)
                    {
                        strncpy(recvdata.device_id, json_device_id->valuestring, MAX_DEVICE_ID_LEN);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: device_id = %s", __func__, __LINE__, recvdata.device_id);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: device_id value is not string", __func__, __LINE__);
                    }

                    json_device_type = cJSON_GetObjectItem(json, "device_type");

                    if (NULL == json_device_type)
                    {
                        cloudc_debug("%s[%d]: failed to get device_type from json", __func__, __LINE__);
                    }
                    else if (json_device_type->type == cJSON_String)
                    {
                        strncpy(recvdata.device_type, json_device_type->valuestring, MAX_DEVICE_TYPE_LEN - 1);
                        recv_status_count++;
                        cloudc_debug("%s[%d]: device_type = %s", __func__, __LINE__, recvdata.device_type);  
                    }
                    else
                    {
                        cloudc_error("%s[%d]: device_type value is not string", __func__, __LINE__);
                    }

                    json_config = cJSON_GetObjectItem(json, "config");

                    if(json_config == NULL)
                    {
                        cloudc_debug("%s[%d]: failed to get config from json", __func__, __LINE__);

                    }
                    else
                    {
                        int i = 0;
                        recv_status_count++;
                        char *tmp = NULL;
                        int keyname_count = 0;

                        for(i = 0; i < 3; i ++) //set as 3, because its type is led, will opti later
                        {
                            cJSON *json_keyNode = cJSON_GetObjectItem(json_config,keyName[i]);
                            cloudc_debug("%s[%d]: keyName[%d] = %s", __func__, __LINE__, i, keyName[i]);
                            if(json_keyNode == NULL)
                            {
                                cloudc_debug("%s[%d]: cannot find %s key in led type\n", __func__, __LINE__, keyName[i]);
                            }
                            else
                            {
                                cloudc_get_ipk_name(head, keyName[i], json_keyNode->valuestring);
                                keyname_count ++;
                            }
                        }
                        cloudc_debug("%s[%d]: keyname_count = %d\n", __func__, __LINE__, keyname_count);
                        recvdata.real_ipk_num = keyname_count;
                        recvdata.ipk_name_head = head;
                    }

                    if (6 == recv_status_count)
                    {
                        rsp_status = 1;
                        cloudc_send_alljoyn_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, recvdata.user_id, recvdata.device_id, recvdata.device_type, rsp_status);
                        cloudc_debug("%s[%d]: correct parameter, will continue to handle it", __func__, __LINE__);  

                        task_queue_enque(&queue_head, recvdata);
                    }
                    else
                    {
                        rsp_status = 0;
                        cloudc_send_alljoyn_recv_rsp_buf(recvdata.rpc_cmd, recvdata.serial_num, recvdata.user_id, recvdata.device_id, recvdata.device_type, rsp_status);
                        cloudc_error("%s[%d]: wrong parameter, no need to handle it", __func__, __LINE__);  
                    }

                    break;
                default:
                    cloudc_error("%s[%d]: unknown flag, recvdata.rpc_cmd = %s, recvdata.rpc_flag = %d", __func__, __LINE__, recvdata.rpc_cmd, recvdata.rpc_flag);
                    break;
            }

        }  
        else
        {
            cloudc_error("%s[%d]: type value is null or not string");

        }
        /* free */
        cJSON_Delete(json);  
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;
}

int cloudc_parse_alljoyn_notification(const char *json_buf)
{
    cJSON *json;

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    cloudc_debug("%s[%d]: json_buf = %s", __func__, __LINE__, json_buf);

    /* parse json_buf */
    json = cJSON_Parse(json_buf);  

    if (!json)  
    {   
        cloudc_debug("%s[%d]: not json format, no need to continue ", __func__, __LINE__);
        cloudc_error("%s[%d]: Error before: [%s]\n", __func__, __LINE__, cJSON_GetErrorPtr());  
    }   
    else
    {
        cloudc_debug("%s[%d]: can go on parsing, will perfect later ", __func__, __LINE__);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;
}

int cloudc_get_type(char *op_type)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if (NULL == op_type)
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return -1;
    }

    strncpy(recvdata.rpc_cmd, op_type, sizeof(recvdata.rpc_cmd) - 1);

    if (!strcmp(op_type, CMD_RSP_REGISTER))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eRegister;
    }

    else if (!strcmp(op_type, CMD_GET_INSTALL_IPK) || !strcmp(op_type, CMD_GET_RUNNING_IPK))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eQueryOperation;
    }

    else if (!strcmp(op_type, CMD_OPKG_UPDATE))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eOpkgOperation;
    }

    else if (!strcmp(op_type, CMD_START_IPK_SERVICE) || !strcmp(op_type, CMD_STOP_IPK_SERVICE) 
            || !strcmp(op_type, CMD_ENABLE_IPK_SERVICE) || !strcmp(op_type, CMD_DISABLE_IPK_SERVICE))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eServiceOperation;
    }

    else if (!strcmp(op_type, CMD_INSTALL_IPK) || !strcmp(op_type, CMD_UPGRADE_IPK) 
            || !strcmp(op_type, CMD_UNINSTALL_IPK))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eIpkOperation;
    }

    else if (!strcmp(op_type, CMD_ALLJOYN_GET_OPERATION))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eAlljoynGetOperation;
    }

    else if (!strcmp(op_type, CMD_ALLJOYN_SET_OPERATION))
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eAlljoynSetOperation;
    }

    else
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return eInvalidParam;
    }
}

int cloudc_get_serial_num(int serial_num)
{
    return serial_num;
}

void cloudc_get_ipk_name(struct ipk_info *list_head, char *ipk_name, char *node_value)
{
    struct ipk_info *ptr_new = NULL;
    struct ipk_info *ptr_tmp = NULL;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: ipk_name = %s, node_value = %s\n", __func__, __LINE__, ipk_name, node_value);

    ptr_new = (struct ipk_info *)malloc(sizeof(struct ipk_info));

    if (NULL == ptr_new)
    {
        cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    memset(ptr_new, 0, sizeof(struct ipk_info));

    strncpy( ptr_new->op_ipk_name, ipk_name, MAX_IPK_NAME_LEN - 1);
    
    if(node_value != NULL)
    {
        strncpy( ptr_new->node_value, node_value, MAX_NODE_VALUE_LEN - 1);
    }

    cloudc_debug("%s[%d]: ptr_new->op_ipk_name = %s, ptr_new->node_value = %s", __func__, __LINE__, ptr_new->op_ipk_name, ptr_new->node_value);

#if 0 /* insert in head */
    if ( NULL == list_head)
    {
        list_head = ptr_new;
        list_head->next = NULL;
    }
    else
    {
        ptr_new->next = list_head;
        list_head = ptr_new;
    }
#endif

    /* insert form tail */
    if ( NULL == list_head)
    {
        list_head = ptr_new;
        list_head->next = NULL;
    }
    else
    {
        ptr_tmp = list_head;

        while( NULL != ptr_tmp->next)
        {
            ptr_tmp = ptr_tmp->next;
        }

        ptr_tmp->next = ptr_new;
        ptr_new->next = NULL;
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

void cloudc_print_ipk_name_list(struct ipk_info *list_head)
{
    struct ipk_info *ptr;
    int j = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if(NULL == list_head)
    {
        cloudc_debug("%s[%d]: no record", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }
    else
    {
        ptr = list_head;
        cloudc_debug("%s[%d]: info as below: \n", __func__, __LINE__);

        while( NULL != ptr)
        {
            cloudc_debug("%s[%d]: ipk name[%d] = %s", __func__, __LINE__, j, ptr->op_ipk_name);
            j = j + 1;
            ptr = ptr->next;
        }
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }
}

void cloudc_destroy_ipk_name(struct ipk_info *list_head)
{
    struct ipk_info *ptr_tmp1;
    struct ipk_info *ptr_tmp2;

    struct ipk_info *tmp_head = list_head;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if( NULL == tmp_head)
    {
        cloudc_debug("%s[%d]no node to delete", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    while( NULL != tmp_head->next)
    {
        ptr_tmp1 = tmp_head->next;
        tmp_head->next = ptr_tmp1->next;
        free(ptr_tmp1);
    }
    free(tmp_head);
    tmp_head = NULL;


    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return;
}

char *define_data_by_device_type(char *device_type)
{
    if(strcmp(device_type, "led") == 0)
    {
        //define the data here
    }
    return NULL;
}

#include "share.h"
#include "parser.h"
#include "cJSON.h"

int cloudc_parse_receive_info(char *recvbuf);
int cloudc_get_type(char *op_type);
int cloudc_get_serial_num(int serial_num);
int cloudc_parse_http_header(char *recvbuf);
char *cloudc_get_http_body(char *recvbuf);
int cloudc_parse_http_body(char *json_buf);
void cloudc_get_ipk_name( char *ipk_name);
void cloudc_print_ipk_name_list(void);
void cloudc_destroy_ipk_name(void);

http_value recvdata;
int rsp_status = 0;
struct ipk_info *head;

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
    int recv_status_count = 0;
    int i = 0;
    eOpType flag;

    char *array_item = NULL;
    char array_item_tmp[MAX_IPK_NAME_LEN] = {0};

    cJSON *json, *json_type, *json_serial, *json_status, *json_package, *json_update, *json_url;
    cJSON *p_array_item = NULL;

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    cloudc_debug("%s[%d]: json_buf = %s", __func__, __LINE__, json_buf);

    /* parse json_buf */
    json = cJSON_Parse(json_buf);  

    if (!json)  
    {   
        cloudc_debug("%s[%d]: not jason format ", __func__, __LINE__);
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
                                cloudc_get_ipk_name(array_item_tmp);
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
                                cloudc_get_ipk_name(array_item_tmp);
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

void cloudc_get_ipk_name( char *ipk_name)
{
    struct ipk_info *ptr_new;
    struct ipk_info *ptr_tmp;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    
    ptr_new = (struct ipk_info *)malloc(sizeof(struct ipk_info));

    if (NULL == ptr_new)
    {
        cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    strncpy( ptr_new->op_ipk_name, ipk_name, MAX_IPK_NAME_LEN - 1);
    cloudc_debug("%s[%d]: ptr_new->op_ipk_name = %s", __func__, __LINE__, ptr_new->op_ipk_name);

#if 0 /* insert in head */
    if ( NULL == head)
    {
        head = ptr_new;
        head->next = NULL;
    }
    else
    {
        ptr_new->next = head;
        head = ptr_new;
    }
#endif

    /* insert form tail */
    if ( NULL == head)
    {
        head = ptr_new;
        head->next = NULL;
    }
    else
    {
        ptr_tmp = head;

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

void cloudc_print_ipk_name_list(void)
{
    struct ipk_info *ptr;
    int j = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if(NULL == head)
    {
        cloudc_debug("%s[%d]: no record", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }
    else
    {
        ptr = head;
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

void cloudc_destroy_ipk_name(void)
{
    struct ipk_info *ptr_tmp1;
    struct ipk_info *ptr_tmp2;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if( NULL == head)
    {
        cloudc_debug("%s[%d]no node to delete", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    if( NULL == head->next)
    {
        free(head);
        head = NULL;
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    ptr_tmp1 = head->next;
    while( NULL != ptr_tmp1)
    {
        ptr_tmp2 = ptr_tmp1;
        ptr_tmp1 = ptr_tmp1->next;
        free(ptr_tmp2);
    }
    free(head);
    head = NULL;

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}


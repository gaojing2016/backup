
/* This file is used to buid packet as defined and send to server.
 * */
#include "share.h"
#include "parser.h"
#include "handle_sys_feature.h"
#include "cJSON.h"

#if 0
/* Below postheader is just for debug, will modify again on official release */
#define POSTHEADER_WITHOUT_CONTENT "POST /test HTTP/1.1\r\n\
HOST:192.168.3.19:3000\r\n\
Connection: close\r\n\
Content-Type: application/json\r\n\
Content-Length: %d\r\n\r\n"
#endif

#define POSTHEADER_WITHOUT_CONTENT "POST / HTTP/1.1\r\n\
Host: %s:%d\r\n\
User-Agent: Mozilla/5.0\r\n\
Accept: text/html, application/json\r\n\
Accept-Language: en-US\r\n\
Accept-Encoding: gzip, deflate\r\n\
Connection: keep-alive\r\n\
Content-Type: application/json\r\n\
Content-Length: %d\r\n\
\r\n"

#if 0
#define POSTHEADER_WITHOUT_CONTENT "POST / HTTP/1.1\r\n\
Host: 192.168.1.10:8888\r\n\
User-Agent: Mozilla/5.0\r\n\
Accept: text/html, application/json\r\n\
Accept-Language: en-US\r\n\
Accept-Encoding: gzip, deflate\r\n\
Connection: keep-alive\r\n\
Content-Type: application/json\r\n\
Content-Length: %d\r\n\
\r\n"
#endif


int cloudc_build_register_js_buf(char *js_buf); 
int cloudc_build_recv_rsp_js_buf(char *type, int serial, int rsp_status, char *js_buf);
int cloudc_build_rsp_ipk_js_buf(char *type, int serial, struct ipk_info *ipk_list_head, int *status, int real_ipk_num, char *js_buf);
int cloudc_build_rsp_query_js_buf(char *type, int serial, struct ipk_query_info_node *query_list_head, char *js_buf);
int cloudc_build_rsp_opkg_js_buf(char *type, int serial, int update_status, int replace_status, char *js_buf);
int cloudc_build_send_http_buf(char *js_buf, char *http_buf);

int cloudc_send_register_buf(void); 
int cloudc_send_recv_rsp_buf(char *type, int serial, int rsp_status);
int cloudc_send_http_buf(char *http_buf);
int cloudc_send_rsp_ipk_buf(char *type, int serial, struct ipk_info *ipk_list_head, int *status, int real_ipk_num);
int cloudc_send_rsp_query_buf(char *type, int serial, struct ipk_query_info_node *query_list_head);
int cloudc_send_rsp_opkg_buf(char *type, int serial, int update_status, int replace_status); 

int cloudc_build_send_http_buf(char *js_buf, char *http_buf)
{
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);

    snprintf(http_buf, SEND_MAX_BUF_LEN, POSTHEADER_WITHOUT_CONTENT, CLOUD_SERVER_MANAGE_IP, CLOUD_SERVER_MANAGE_PORT, strlen(js_buf));
    strncat(http_buf, js_buf, (SEND_MAX_BUF_LEN - strlen(http_buf)));

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;
}

int cloudc_send_http_buf(char *http_buf)
{
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    int ret = -1;

    if (-1 == is_socket_connected(cloudc_monitor_uloop_fd.fd))
    {
        close(cloudc_monitor_uloop_fd.fd);
        socket_init();
        ap_register();
    }

    if (NULL != http_buf)
    {
        pthread_mutex_lock(&mutex);
        //ret = send(cloudc_monitor_uloop_fd.fd, http_buf, SEND_MAX_BUF_LEN, 0);
        ret = send(cloudc_monitor_uloop_fd.fd, (char *)http_buf, strlen(http_buf), 0);
        //uloop_timeout_set(&send_timer, SEND_TIME_OUT * 1000);

        pthread_mutex_unlock(&mutex);

        if (ret > 0)
        {
            cloudc_debug("%s[%d]: ret = %d, client send http buf succeed", __func__, __LINE__, ret);
            cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
            return 0;
        }
        else if (ret = -1)
        {
            cloudc_error("%s[%d]: send() failed: ret = %d, %s!", __func__, __LINE__, ret, strerror(errno));
            close(cloudc_monitor_uloop_fd.fd);
            socket_create();
            socket_connect();
            ap_register();
            cloudc_send_http_buf(http_buf);
        }

        else
        {
            cloudc_error("%s[%d]: send() failed: ret = %d, %s!", __func__, __LINE__, ret, strerror(errno));
            cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
            return -1;
        }
    }

    else
    {
        cloudc_error("%s[%d]: http_buf is null, please double confirm!", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
        return -1;
    }
}

int cloudc_send_rsp_ipk_buf(char *type, int serial, struct ipk_info *ipk_list_head, int *status, int real_ipk_num)
{
    cloudc_error("%s[%d]: Enter!", __func__, __LINE__);
    char build_js_buf[SEND_MAX_BUF_LEN] = {0};
    char build_http_buf[SEND_MAX_BUF_LEN] = {0};
    int ret1 = -1;

    ret1 = cloudc_build_rsp_ipk_js_buf(type, serial, ipk_list_head, status, real_ipk_num, build_js_buf);

    if (0 == ret1)
    {
        /* gaojing: need to sprintf http package then send */
        cloudc_build_send_http_buf(build_js_buf, build_http_buf);
        cloudc_debug("%s[%d]: build_js_buf = %s, \nbuild_http_buf = %s", __func__, __LINE__, build_js_buf, build_http_buf);

        cloudc_send_http_buf(build_http_buf);

    }

    cloudc_error("%s[%d]: Exit!", __func__, __LINE__);
    return ret1;
}

int cloudc_send_rsp_query_buf(char *type, int serial, struct ipk_query_info_node *query_list_head)
{
    cloudc_error("%s[%d]: Enter!", __func__, __LINE__);
    char build_js_buf[SEND_MAX_BUF_LEN] = {0};
    char build_http_buf[SEND_MAX_BUF_LEN] = {0};
    int ret1 = -1;

    ret1 = cloudc_build_rsp_query_js_buf(type, serial, query_list_head, build_js_buf);

    if (0 == ret1)
    {
        /* gaojing: need to sprintf http package then send */
        cloudc_build_send_http_buf(build_js_buf, build_http_buf);
        cloudc_debug("%s[%d]: build_js_buf = %s, \nbuild_http_buf = %s", __func__, __LINE__, build_js_buf, build_http_buf);

        cloudc_send_http_buf(build_http_buf);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return ret1;
}

int cloudc_build_rsp_query_js_buf(char *type, int serial, struct ipk_query_info_node *query_list_head, char *js_buf)
{
    struct ipk_query_info_node *query_list_tmp = query_list_head->pNext;
    cJSON *root;
    /*create json string root*/
    root = cJSON_CreateObject();

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);

    if (!root) 
    {   
        cloudc_debug("%s[%d]: get root faild !", __func__, __LINE__);
        cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
        goto EXIT;
    }   
    else 
    {   
        cloudc_debug("%s[%d]: get root success! ", __func__, __LINE__);
    }   

    {   
        cJSON * js_body ;
        char rsp_type[40] = {0};
        snprintf(rsp_type, sizeof(rsp_type), "rsp_%s", type);

        const char *const body = "status"; 
        cJSON_AddStringToObject(root, "type", rsp_type); 
        cJSON_AddNumberToObject(root, "serial", serial); 
        cJSON_AddItemToObject(root, body, js_body= cJSON_CreateArray()); 

        while (NULL != query_list_tmp)
        {
            cJSON_AddStringToObject(js_body, "name", query_list_tmp->ipk_query_name);
            query_list_tmp = query_list_tmp->pNext;
        }

        {
            char *s = cJSON_PrintUnformatted(root);
            if (s)
            {
                strncpy(js_buf, s, SEND_MAX_BUF_LEN - 1);
                cloudc_debug("%s[%d]: create js_buf  is %s\n", __func__, __LINE__, js_buf);
                free(s);
            }
        }
        cJSON_Delete(root);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;
EXIT:
    return -1;
}

int cloudc_build_rsp_ipk_js_buf(char *type, int serial, struct ipk_info *ipk_list_head, int *status, int real_ipk_num, char *js_buf)
{
    cloudc_error("%s[%d]: Enter!", __func__, __LINE__);
    cJSON *json_root;
    struct ipk_info *ipk_list_tmp = ipk_list_head;

    /*create json string root*/
    json_root = cJSON_CreateObject();

    if (!json_root)
    {
        cloudc_error("%s[%d]: get json_root faild !\n", __func__, __LINE__);
        goto EXIT;
    }

    else
    {
        cloudc_debug("%s[%d]: get json_root succeed !\n", __func__, __LINE__);
    }

    {
        cJSON * js_array, *js_body ;

        char rsp_type[30] = {0};
        snprintf(rsp_type, sizeof(rsp_type), "rsp_%s", type);

        cJSON_AddStringToObject(json_root, "type", rsp_type);
        cJSON_AddNumberToObject(json_root, "serial", serial);
        cJSON_AddItemToObject(json_root, "status", js_array= cJSON_CreateArray());

        int i = 0;
        for (i = 0; i < real_ipk_num; i ++)
        {            
            cJSON_AddItemToArray(js_array, js_body = cJSON_CreateObject());
            cJSON_AddItemToObject(js_body, "name", cJSON_CreateString(ipk_list_tmp->op_ipk_name));
            cJSON_AddItemToObject(js_body, "st", cJSON_CreateNumber(status[i]));
            cloudc_debug("%s[%d]: ############# ipk_name[%d] = %s, status[%d] = %d", __func__, __LINE__, i, ipk_list_tmp->op_ipk_name, i, status[i]);
            ipk_list_tmp = ipk_list_tmp->next;
        }

        {
            char *s = cJSON_PrintUnformatted(json_root);

            if (s)
            {
                strncpy(js_buf, s, SEND_MAX_BUF_LEN - 1);
                cloudc_debug("%s[%d]: create js_buf  is %s\n", __func__, __LINE__, js_buf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }

    return 0;
EXIT:
    return -1;
}


int cloudc_build_register_js_buf(char *js_buf)
{
    cJSON *json_root;
    /*create json string root*/
    json_root = cJSON_CreateObject();

    if (!json_root) 
    {   
        cloudc_debug("%s[%d]: get json_root faild !", __func__, __LINE__);
        goto EXIT;
    }   
    else 
    {   
        cloudc_debug("%s[%d]: get json_root success!", __func__, __LINE__);
    }   

    {   
        cJSON_AddStringToObject(json_root, "type", "register"); 
        cJSON_AddNumberToObject(json_root, "serial", 0); // this message is sent by client, how I can get the serial num, or set it as 0 default?
        cJSON_AddStringToObject(json_root, "sn", ap_sn); 
        cJSON_AddStringToObject(json_root, "mac", ap_mac); 

        {   
            char *s = cJSON_PrintUnformatted(json_root);
            if (s)
            {
                strncpy(js_buf, s, SEND_MAX_BUF_LEN - 1);
                cloudc_debug("%s[%d]: create js_buf  is %s\n", __func__, __LINE__, js_buf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }
    return 0;
EXIT:
    return -1;
}

int cloudc_send_register_buf(void)
{   
    char build_js_buf[SEND_MAX_BUF_LEN] = {0};
    char build_http_buf[SEND_MAX_BUF_LEN] = {0};
    int ret1 = -1;
    
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);

    ret1 = cloudc_build_register_js_buf(build_js_buf);

    if (0 == ret1)
    {
        /* gaojing: need to sprintf http package then send */
        cloudc_build_send_http_buf(build_js_buf, build_http_buf);
        cloudc_debug("%s[%d]: build_js_buf = %s, \nbuild_http_buf = %s", __func__, __LINE__, build_js_buf, build_http_buf);

        cloudc_send_http_buf(build_http_buf);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return ret1;
}

int cloudc_build_recv_rsp_js_buf(char *type, int serial, int rsp_status, char *js_buf)
{
    cJSON *json_root;
    /*create json string root*/
    json_root = cJSON_CreateObject();

    char rsp_type[30] = {0};
    snprintf(rsp_type, sizeof(rsp_type), "rsp_%s", type);

    if (!json_root) 
    {   
        cloudc_debug("%s[%d]: get json_root faild !", __func__, __LINE__);
        goto EXIT;
    }   
    else 
    {   
        cloudc_debug("%s[%d]: get json_root success, rsp_status = %d!", __func__, __LINE__, rsp_status);
    }   

    {   
        cJSON_AddStringToObject(json_root, "type", rsp_type); 
        cJSON_AddNumberToObject(json_root, "serial", serial); 
        cJSON_AddNumberToObject(json_root, "status", rsp_status); 
        {   
            char *s = cJSON_PrintUnformatted(json_root);
            if (s)
            {
                strncpy(js_buf, s, SEND_MAX_BUF_LEN - 1);
                cloudc_debug("%s[%d]: create js_buf  is %s\n", __func__, __LINE__, js_buf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }
    return 0;
EXIT:
    return -1;
}

int cloudc_send_recv_rsp_buf(char *type, int serial, int rsp_status)
{   
    char build_js_buf[SEND_MAX_BUF_LEN] = {0};
    char build_http_buf[SEND_MAX_BUF_LEN] = {0};
    int ret1 = -1;

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);


    ret1 = cloudc_build_recv_rsp_js_buf(type, serial, rsp_status, build_js_buf);

    if (0 == ret1)
    {
        /* gaojing: need to sprintf http package then send */
        cloudc_build_send_http_buf(build_js_buf, build_http_buf);
        cloudc_debug("%s[%d]: build_js_buf = %s, \nbuild_http_buf = %s", __func__, __LINE__, build_js_buf, build_http_buf);

        cloudc_send_http_buf(build_http_buf);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return ret1;
}

int cloudc_send_rsp_opkg_buf(char *type, int serial, int update_status, int replace_status)
{
    char build_js_buf[SEND_MAX_BUF_LEN] = {0};
    char build_http_buf[SEND_MAX_BUF_LEN] = {0};
    int ret1 = -1;
    
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);


    ret1 = cloudc_build_rsp_opkg_js_buf(type, serial, update_status, replace_status, build_js_buf);

    if (0 == ret1)
    {
        /* gaojing: need to sprintf http package then send */
        cloudc_build_send_http_buf(build_js_buf, build_http_buf);
        cloudc_debug("%s[%d]: build_js_buf = %s, \nbuild_http_buf = %s", __func__, __LINE__, build_js_buf, build_http_buf);

        cloudc_send_http_buf(build_http_buf);
    }
    return ret1;
}

int cloudc_build_rsp_opkg_js_buf(char *type, int serial, int update_status, int replace_status, char *js_buf)
{
    cJSON *json_root;
    /*create json string root*/
    json_root = cJSON_CreateObject();

    char rsp_type[30] = {0};
    snprintf(rsp_type, sizeof(rsp_type), "rsp_%s", type);

    if (!json_root) 
    {   
        cloudc_debug("%s[%d]: get json_root faild !", __func__, __LINE__);
        goto EXIT;
    }   
    else 
    {   
        cloudc_debug("%s[%d]: get json_root success!", __func__, __LINE__);
    }   

    {   
        cJSON_AddStringToObject(json_root, "type", rsp_type); 
        cJSON_AddNumberToObject(json_root, "serial", serial); 
        cJSON_AddNumberToObject(json_root, "update", update_status); 
        cJSON_AddNumberToObject(json_root, "url", replace_status); 
        {   
            char *s = cJSON_PrintUnformatted(json_root);
            if (s)
            {
                strncpy(js_buf, s, SEND_MAX_BUF_LEN - 1);
                cloudc_debug("%s[%d]: create js_buf  is %s\n", __func__, __LINE__, js_buf);
                free(s);
            }
        }
        cJSON_Delete(json_root);
    }
    return 0;
EXIT:
    return -1;

}


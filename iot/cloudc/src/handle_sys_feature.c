#include "share.h"
#include "parser.h"
#include "handle_sys_feature.h"
#include <curl/curl.h>

int get_ap_info(void);
void cloudc_rpc_method_handle(struct http_value recv_data);
int cloudc_manage_ipk(char *op_type, int serial_num, struct ipk_info *ipk_list_head, int real_ipk_num);
int cloudc_manage_query(char *op_type, int serial_num);
int cloudc_manage_opkg(char *op_type, int serial_num, int update_flag, char *url);
int cloudc_manage_service(char *op_type, int serial_num, struct ipk_info *ipk_list_head, int real_ipk_num);
int cloudc_manage_alljoyn_get_operation(char *op_type, int serial_num, char *user_id, char *device_id, char *device_type, struct ipk_info *name_list_head, int real_ipk_num);
int cloudc_manage_alljoyn_set_operation(char *op_type, int serial_num, char *user_id, char *device_id, char *device_type, struct ipk_info *name_list_head, int real_ipk_num);

int cloudc_download_file(char *download_url);
int cloudc_opkg_conf_update();

int cloudc_install_ipk(char *ipk_name);
int cloudc_upgrade_ipk(char *ipk_name);
int cloudc_uninstall_ipk(char *ipk_name);
int check_ipk_installed(char *ipk_name);

int cloudc_get_running_ipk_list(char *op_type, int serial_num);
int cloudc_get_install_ipk_list(char *op_type, int serial_num);

int cloudc_start_ipk(char *ipk_name);
int cloudc_stop_ipk(char *ipk_name);
int cloudc_enable_ipk(char *ipk_name);
int cloudc_disable_ipk(char *ipk_name);


static size_t my_fwrite(void *buffer, size_t size, size_t nmemb,
        void *stream)
{
    struct FtpFile *out=(struct FtpFile *)stream;
    if(out && !out->stream) {
        /* open file for writing */
        out->stream=fopen(out->filename, "wb");
        if(!out->stream)
            return -1; /* failure, can't open file to write */
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

int get_ap_info(void)
{
    /* need to add how to get ap mac here */
    char temp_ap_mac[18] = "42:21:33:dd:08:08";
    char temp_ap_sn[40] = "tempserialnumber0099";
    strncpy(ap_mac, temp_ap_mac, sizeof(ap_mac) - 1);
    strncpy(ap_sn, temp_ap_sn, sizeof(ap_mac) - 1);
    return 0;
}

void cloudc_rpc_method_handle(struct http_value recv_data)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    switch(recv_data.rpc_flag)
    {
        case eQueryOperation:
            cloudc_manage_query(recv_data.rpc_cmd, recv_data.serial_num);
            break;

        case eIpkOperation:
            cloudc_manage_ipk(recv_data.rpc_cmd, recv_data.serial_num, recv_data.ipk_name_head->next, recv_data.real_ipk_num);
            break;

        case eOpkgOperation:
            cloudc_manage_opkg(recv_data.rpc_cmd, recv_data.serial_num, recv_data.opkg_update_flag, recv_data.opkg_update_url);
            break;

        case eServiceOperation:
            cloudc_manage_service(recv_data.rpc_cmd, recv_data.serial_num, recv_data.ipk_name_head->next, recv_data.real_ipk_num);
            break;

        case eRegister:
            ap_register_flag = recv_data.register_status;
#if 0
            if (1 == recv_data.register_status)
            {
                /* status = 1 means register fail
                 * need to send register request again
                 * */
            }
            else if ((0 == recv_data.register_status) || (2 == recv_data.register_status))
            {
                /* status = 0 means register succeed
                 * status = 2 meand already registered
                 * */
            }
#endif
            break;

        case eAlljoynGetOperation:
            cloudc_manage_alljoyn_get_operation(recv_data.rpc_cmd, recv_data.serial_num, recv_data.user_id, recv_data.device_id, recv_data.device_type, recv_data.ipk_name_head->next, recv_data.real_ipk_num);
            break;

        case eAlljoynSetOperation:
            cloudc_manage_alljoyn_set_operation(recv_data.rpc_cmd, recv_data.serial_num, recv_data.user_id, recv_data.device_id, recv_data.device_type, recv_data.ipk_name_head->next, recv_data.real_ipk_num);
            break;

        default:
            cloudc_error("%s[%d]: unknow flag, rpc_flag = %d", __func__, __LINE__, recv_data.rpc_flag);
            break;
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
}

int cloudc_manage_opkg(char *op_type, int serial_num, int update_flag, char *url)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    int update_status = -1;
    int replace_status = -1;

    if (NULL != url)
    {
        /*
         * 1.download new opkg_conf from this url
         * 2.replace opkg.conf
         * */
        replace_status = cloudc_download_file(url);

        /* 0 means succeed, 1 means failed */
    }
    else
    {
        replace_status = 0; /* no need replace, so set it as 0 */
        cloudc_debug("%s[%d]: no need to replace opkg_conf file", __func__, __LINE__);
    }

    if (1 == update_flag)
    {
        /* need update */
        /* 0 means succeed, 1 means failed */
        update_status = cloudc_opkg_conf_update();
    }
    else
    {
        update_status = 0; /* no need update, so set it as 0 */
        cloudc_debug("%s[%d]: no need to update", __func__, __LINE__);
    }

    cloudc_send_rsp_opkg_buf(op_type, serial_num, update_status, replace_status);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_manage_query(char *op_type, int serial_num)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    if (0 == strcmp(op_type, CMD_GET_INSTALL_IPK))
    {
        cloudc_get_install_ipk_list(op_type, serial_num);
    }

    else if (0 == strcmp(op_type, CMD_GET_RUNNING_IPK))
    {
        cloudc_get_running_ipk_list(op_type, serial_num);
    }
    else
    {
        cloudc_debug("%s[%d]: unknown type", __func__, __LINE__);
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);

    return 0;
}

int cloudc_manage_ipk(char *op_type, int serial_num, struct ipk_info *ipk_list_head, int real_ipk_num)
{
    int status[real_ipk_num];
    int i = 0;
    struct ipk_info *ipk_list_tmp = ipk_list_head;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    memset(status, -1, sizeof(status));

    if (0 == strcmp(op_type, CMD_INSTALL_IPK))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_install_ipk(ipk_list_tmp->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to install %s", __func__, __LINE__, ipk_list_tmp->op_ipk_name);
            }

            ipk_list_tmp = ipk_list_tmp->next;
        }

        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else if (0 == strcmp(op_type, CMD_UNINSTALL_IPK))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_uninstall_ipk(ipk_list_tmp->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to uninstall %s", __func__, __LINE__, ipk_list_tmp->op_ipk_name);
            }

            ipk_list_tmp = ipk_list_tmp->next;
        }

        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else if (0 == strcmp(op_type, CMD_UPGRADE_IPK))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_upgrade_ipk(ipk_list_tmp->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to upgrade %s", __func__, __LINE__, ipk_list_tmp->op_ipk_name);
            }

            ipk_list_tmp = ipk_list_tmp->next;
        }
        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else
    {
        cloudc_error("%s[%d]: invaild op_type, need server confirm", __func__, __LINE__);
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);

    return 0;
}

int cloudc_manage_service(char *op_type, int serial_num,  struct ipk_info *ipk_list_head, int real_ipk_num)
{
    int status[real_ipk_num];
    int i = 0;
    struct ipk_info *ipk_list_tmp = ipk_list_head;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    memset(status, -1, sizeof(status));

    if (0 == strcmp(op_type, CMD_START_IPK_SERVICE))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_start_ipk(ipk_list_tmp->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to start %s service", __func__, __LINE__, ipk_list_head->op_ipk_name);
            }

            ipk_list_tmp = ipk_list_tmp->next;

        }
        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else if (0 == strcmp(op_type, CMD_STOP_IPK_SERVICE))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_stop_ipk(ipk_list_head->op_ipk_name);
            {
                cloudc_error("%s[%d]: failed to start %s service", __func__, __LINE__, ipk_list_head->op_ipk_name);
            }

            ipk_list_tmp = ipk_list_tmp->next;
        }
        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else if (0 == strcmp(op_type, CMD_ENABLE_IPK_SERVICE))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_enable_ipk(ipk_list_head->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to enable %s service", __func__, __LINE__, ipk_list_head->op_ipk_name);
            }

        }
        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else if (0 == strcmp(op_type, CMD_DISABLE_IPK_SERVICE))
    {
        for (i = 0; i < real_ipk_num; i ++)
        {
            status[i] = cloudc_disable_ipk(ipk_list_head->op_ipk_name);
            if (-1 == status[i])
            {
                cloudc_error("%s[%d]: failed to disable %s service", __func__, __LINE__, ipk_list_head->op_ipk_name);
            }
        }
        cloudc_send_rsp_ipk_buf(op_type, serial_num, ipk_list_head, status, real_ipk_num);
    }

    else
    {
        cloudc_error("%s[%d]: invaild op_type, need server confirm", __func__, __LINE__);
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);

    return 0;
}

int cloudc_manage_alljoyn_get_operation(char *op_type, int serial_num, char *user_id, char *device_id, char *device_type, struct ipk_info *name_list_head, int real_ipk_num)
{
    char objectPath[MAX_OBJECTPATH_LEN] = {0};
    char interfaceName[MAX_INTERFACE_LEN] = {0};
    char onlineStatus[16] = {0};
    char platform[10] = "wechat";
    struct ipk_info *tmp_name_list_head = name_list_head;
    int msg_type = -1;
    int ret = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    cloudc_debug("%s[%d]: op_type = %s, serial_num = %d, user_id = %s, device_id = %s, device_type = %s, real_ipk_num = %d\n", __func__, __LINE__, op_type, serial_num, user_id, device_id, device_type, real_ipk_num);

    if(strcmp(op_type, "get") == 0)
    {
        msg_type = 0;
    }
    else if(strcmp(op_type, "set") == 0)
    {
        msg_type = 1;
    }

    getObjInfoByDevId(device_id, interfaceName, objectPath, device_type, onlineStatus);
    if(strcmp(onlineStatus, "on") == 0)
    {
        ret = getAlljoynKeyName(device_type, platform, name_list_head, real_ipk_num);
        
        if(ret == 0)
        {
            cloudc_debug("%s[%d]: interfaceName = %s, objectPath = %s\n", __func__, __LINE__, interfaceName, objectPath);

            if((strcmp(interfaceName, "") != 0) && (strcmp(objectPath, "") != 0))
            {
                configClientMain(interfaceName,objectPath, msg_type, name_list_head, real_ipk_num);
            }

            while(tmp_name_list_head != NULL)
            {
                cloudc_debug("%s[%d]: platformkeyname = %s, alljoynKeyname = %s, keyvalue = %s\n", __func__, __LINE__, tmp_name_list_head->op_ipk_name, tmp_name_list_head->node_name_convert, tmp_name_list_head->node_value);
                tmp_name_list_head = tmp_name_list_head->next;
            }

            cloudc_send_rsp_get_buf(op_type, serial_num, user_id, device_id, device_type, name_list_head, real_ipk_num);
        }
        else
        {
            cloudc_debug("%s[%d]: cannot find keyname in DataConvert.xml", __func__, __LINE__);
        }
    }
    else
    {
        cloudc_debug("%s[%d]: device is already offline, cannot be configured", __func__, __LINE__);
        /* to do: need to send smartDevice offline event to server later  */
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_manage_alljoyn_set_operation(char *op_type, int serial_num, char *user_id, char *device_id, char *device_type, struct ipk_info *name_list_head, int real_ipk_num)
{
    char objectPath[MAX_OBJECTPATH_LEN] = {0};
    char interfaceName[MAX_INTERFACE_LEN] = {0};
    char onlineStatus[16] = {0};
    char platform[10] = "wechat";
    int msg_type = -1;
    int ret = 0;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    struct ipk_info *tmp_name_list_head = name_list_head;

    if(strcmp(op_type, "get") == 0)
    {
        msg_type = 0;
    }
    else if(strcmp(op_type, "set") == 0)
    {
        msg_type = 1;
    }

    cloudc_debug("%s[%d]: op_type = %s, serial_num = %d, user_id = %s, device_id = %s, device_type = %s, real_ipk_num = %d\n", __func__, __LINE__, op_type, serial_num, user_id, device_id, device_type, real_ipk_num);

    getObjInfoByDevId(device_id, interfaceName, objectPath, device_type, onlineStatus);
    if(strcmp(onlineStatus, "on") == 0)
    {
        ret = getAlljoynKeyName(device_type, platform, name_list_head, real_ipk_num);

        if(ret == 0)
        {

            cloudc_debug("%s[%d]: interfaceName = %s, objectPath = %s\n", __func__, __LINE__, interfaceName, objectPath);

            if((strcmp(interfaceName, "") != 0) && (strcmp(objectPath, "") != 0))
            {
                configClientMain(interfaceName,objectPath, msg_type, name_list_head, real_ipk_num);
            }
            else
            {
                while(tmp_name_list_head != NULL)
                {
                    memset(tmp_name_list_head->node_value, 0, MAX_NODE_VALUE_LEN);
                    strcpy(tmp_name_list_head->node_value, "setError");
                    tmp_name_list_head->status = 1;
                    cloudc_debug("%s[%d]: keyname = %s, keyvalue = %s\n", __func__, __LINE__, tmp_name_list_head->op_ipk_name, tmp_name_list_head->node_value);
                    tmp_name_list_head = tmp_name_list_head->next;
                }
            }

            cloudc_send_rsp_set_buf(op_type, serial_num, user_id, device_id, device_type, name_list_head, real_ipk_num);
        }
        else
        {
            cloudc_debug("%s[%d]: cannot find keyname in DataConvert.xml", __func__, __LINE__);
        }
    }
    else
    {
        cloudc_debug("%s[%d]: device is already offline, cannot be configured", __func__, __LINE__);
        /* to do: need to send smartDevice offline event to server later  */
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_download_file(char *download_url)
{
    CURL *curl;
    CURLcode res;
    int download_status = -1;
    struct FtpFile ftpfile={
        "/etc/opkg.temp", /* name to store the file as if succesful */
        NULL
    };

    cloudc_debug("%s[%d]: Enter, and url is %s", __func__, __LINE__, download_url);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        /*
         * You better replace the URL with one that works! Note that we use an
         * FTP:// URL with standard explicit FTPS. You can also do FTPS:// URLs if
         * you want to do the rarer kind of transfers: implicit.
         */
        curl_easy_setopt(curl, CURLOPT_URL, download_url);
        /* Define our callback to get called when there's data to be written */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        /* Set a pointer to our struct to pass to the callback */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

        /* Switch on full protocol/debug output */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);

        if(CURLE_OK != res) {
            /* we failed */
            cloudc_debug("%s[%d]: curl told us %d\n", __func__, __LINE__, res);
            download_status = 1;
        }
    }

    if(ftpfile.stream)
        fclose(ftpfile.stream); /* close the local file */

    curl_global_cleanup();

    if(1 == download_status)
    {
        cloudc_debug("%s[%d]: Exit,download opkg conf failure", __func__, __LINE__);
        return 1;
    }
    else
    {
        if(0 == rename("/etc/opkg.temp","/etc/opkg.conf") )
        {
            cloudc_debug("%s[%d]: Exit,rename success", __func__, __LINE__);
            return 0;
        }
        else
        {
            cloudc_debug("%s[%d]: Exit,rename failure", __func__, __LINE__);
            return 1;
        }
    }
}

int cloudc_install_ipk(char *ipk_name)
{
    int package_len = strlen(ipk_name);
    int check_result = -1;

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    if ( NULL != ipk_name)
    {
        char * temp_buffer = (char *)malloc(package_len + LEN_OF_CMD_HEAD);
        if (NULL == temp_buffer)
        {
            cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
            return IPK_UNINSTALLED;
        }
        strcpy(temp_buffer, "opkg-customer install ");
        strcat(temp_buffer, ipk_name);
        system("opkg-customer update");
        system(temp_buffer);

        free(temp_buffer);

        check_result = check_ipk_installed(ipk_name);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    }
    else
    {
        check_result = IPK_UNINSTALLED;
    }

    if (check_result == IPK_INSTALLED)
        return 0;
    else
        return 1;
}

int cloudc_upgrade_ipk(char *ipk_name)
{
    int check_result_1 = -1;
    int check_result_2 = -1;

    if (NULL != ipk_name)
    {
        cloudc_uninstall_ipk(ipk_name);
        check_result_1 = check_ipk_installed(ipk_name);
        cloudc_install_ipk(ipk_name);
        check_result_2 = check_ipk_installed(ipk_name);
    }
    else
    {
        return -1;
    }

    if (check_result_1 == IPK_UNINSTALLED && check_result_2 == IPK_INSTALLED)
        return 0;
    else
        return -1;
}

int cloudc_uninstall_ipk(char *ipk_name)
{
    /* To do: need to check if the ipk is installed firstly
     * if not installed, just return to tell server this is a wrong commond
     * if installed, just excute 
     * */
    int package_len = strlen(ipk_name);
    int check_result = -1;

    if (NULL != ipk_name)
    {
        char * temp_buffer = (char *)malloc(package_len + LEN_OF_CMD_HEAD);
        if (NULL == temp_buffer)
        {
            cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
            return IPK_UNINSTALLED;
        }
        strcpy(temp_buffer, "opkg-customer remove ");
        strcat(temp_buffer, ipk_name);
        system(temp_buffer);
        free(temp_buffer);

        check_result = check_ipk_installed(ipk_name);
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    }
    else
    {
        check_result == IPK_UNINSTALLED;
    }

    if (check_result == IPK_UNINSTALLED)
        return 0;
    else
        return 1;
}

int cloudc_get_running_ipk_list(char *op_type, int serial_num)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    char temp_running_ipk_list[][MAX_IPK_NAME_LEN] = {"portal","loudc","hello"};
    int query_ipk_num = 3;

    cloudc_send_rsp_query_buf(op_type, serial_num, temp_running_ipk_list, query_ipk_num);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);

    return 0;
}

int cloudc_get_install_ipk_list(char *op_type, int serial_num)
{
    FILE *fstream;
    char temp_string[50];

    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    system("opkg-customer list-customer-installed > /tmp/customer_ipk_list");

    fstream = fopen("/tmp/customer_ipk_list", "r");
    if (fstream==NULL)
    {
        cloudc_debug("%s[%d]: fail to open customer_ipk_list", __func__, __LINE__);
        return -1;
    }

    IPK_QUERY_INFO_NODE *pHead = malloc(sizeof(IPK_QUERY_INFO_NODE));
    pHead->pNext = NULL;
    IPK_QUERY_INFO_NODE *pTail = pHead;
    IPK_QUERY_INFO_NODE *pTemp;

    while(fgets(temp_string, 50, fstream) != NULL)
    {
        cloudc_debug("%s[%d]: temp_string is %s", __func__, __LINE__, temp_string);
        if(strlen(temp_string) > MAX_IPK_NAME_LEN)
        {
            cloudc_debug("%s[%d]: ipk_name is too long", __func__, __LINE__);
            return 1;
        }

        pTemp = malloc(sizeof(IPK_QUERY_INFO_NODE));

        temp_string[strlen(temp_string) - 1] = '\0'; /* replace the last '\n' with '0'*/
        cloudc_debug("%s[%d]: temp_string = %s", __func__, __LINE__, temp_string);
        strncpy(pTemp->ipk_query_name, temp_string, MAX_IPK_NAME_LEN - 1);

        pTemp->pNext = NULL;
        pTail->pNext = pTemp;
        pTail = pTemp;
    }
    fclose(fstream);

    cloudc_debug("%s[%d]: prepare to send rsp query buf", __func__, __LINE__);
    cloudc_send_rsp_query_buf(op_type, serial_num, pHead);
    cloudc_debug("%s[%d]: already finish to send rsp query buf", __func__, __LINE__);

    /*free all node*/
    pTemp = pHead->pNext;
    cloudc_debug("%s[%d]: pHead=%p, pTemp=%p", __func__, __LINE__, pHead, pTemp);
    while(pHead != NULL)
    {
        free(pHead);
        pHead = pTemp;
        if(pTemp->pNext != NULL)
            pTemp = pTemp->pNext;
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);

    return 0;
}

int cloudc_start_ipk(char *ipk_name)
{
    char cmd[128] = {0};
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_stop_ipk(char *ipk_name)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_enable_ipk(char *ipk_name)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int cloudc_disable_ipk(char *ipk_name)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return 0;
}

int check_ipk_installed(char *ipk_name)
{
    FILE *fstream;
    char temp_string[50];

    system("opkg-customer list-customer-installed > /tmp/customer_ipk_list");

    fstream = fopen("/tmp/customer_ipk_list", "r");
    if (fstream==NULL)
    {
        cloudc_debug("%s[%d]: fail to open customer_ipk_list", __func__, __LINE__);
        return -1;
    }

    cloudc_debug("%s[%d]: ipk_name is %s", __func__, __LINE__, ipk_name);
    while(fgets(temp_string, 50, fstream) != NULL)
    {
        cloudc_debug("%s[%d]: temp_string is %s", __func__, __LINE__, temp_string);
        if(temp_string == strstr(temp_string, ipk_name))
        {
            cloudc_debug("%s[%d]: ipk installed", __func__, __LINE__);
            fclose(fstream);
            return 0;
        }
    }
    fclose(fstream);
    cloudc_debug("%s[%d]: ipk not installed", __func__, __LINE__);
    return 1;
}

int cloudc_opkg_conf_update()
{
    FILE *fstream;
    int temp;

    system("opkg-customer update 2>/tmp/opkg_update_result");
    fstream = fopen("/tmp/opkg_update_result", "r");
    if (fstream != NULL)
    {
        if((temp=fgetc(fstream)) != EOF)
        {
            cloudc_debug("%s[%d]: opkg update error", __func__, __LINE__);
            fclose(fstream);
            return 1;
        }
    }

    cloudc_debug("%s[%d]: opkg update success", __func__, __LINE__);
    fclose(fstream);
    return 0;
}

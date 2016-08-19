#include "share.h"
#include "parser.h" 
#include "cJSON.h" 

#define NEED_REGISTER 1
#define NO_NEED_REGISTER 0
#define CLOUDC_GET_CONFIG "/etc/config/cloudc"
#define CLOUD_SERVER_URL_LEN 128

/** public data **/
int testcpp = 0;
char ap_mac[18] = {0};
char ap_sn[40] = {0};
char cloud_server_url[CLOUD_SERVER_URL_LEN] = {0};
int cloud_server_port = 0;
int ap_register_flag = 0;
/* attention: 
 * ap_register_flag need to set as 1 on officail release
 * now set it as 0 because it is easy to debug other func
 * */


/** function declare **/
int ap_check_register_condition(void);
int ap_register(void);
int config_init(void);

int ap_check_register_condition(void)
{
    /* This func is used to check whether the ap can register to server, such as below:
     * if wan is up?
     * ...
     * currentl as the interface required, no matter the ap is registered or not before,
     * it will send register request again when ap reboot
     *
     */

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);

    return NEED_REGISTER;
}

int ap_register(void)
{
    /* This func is used to register to cloud server.
     * After registered, ap can get some info which can be used to communicate with cloud server.
     * The info cloud server return may like the cloud_manage_ip, report_ip, report_port....
     * ...
     * if the ap is not registered, then will go to this func.
     * if ap cannot register succeed in this func, just register again and again .....
     *
     */

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    /* need to add how to register ==
     * need to register again and again until it register succeed
     */
    cloudc_send_register_buf();
    uloop_timeout_set(&register_recv_timer, RECV_TIME_OUT * 1000);

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);

    return -1;
}

int config_init(void)
{
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    char *cloud_server_url_new = NULL;
    int register_flag = 0;
    struct uci_context *cloudc_uci_ctx ;
    struct uci_package *cloudc_uci_pkg ;
    struct uci_element *cloudc_uci_ele;
    char  *value;

    get_ap_info();
    cloudc_uci_ctx = uci_alloc_context(); // 申请一个UCI上下文
    if (UCI_OK != uci_load(cloudc_uci_ctx, CLOUDC_GET_CONFIG, &cloudc_uci_pkg))
    {
        cloudc_error("%s[%d]: Failed to uci load", __func__, __LINE__);
        goto cleanup;
    }
    /*遍历UCI的每一个节*/
    uci_foreach_element(&cloudc_uci_pkg->sections,cloudc_uci_ele)
    {
        struct uci_section *cloudc_uci_sec = uci_to_section(cloudc_uci_ele);

        if (NULL != (value = uci_lookup_option_string(cloudc_uci_ctx, cloudc_uci_sec, "loglevel")))
        {

            if (NULL != strcasestr(value, "debug"))
            {
                cloudc_log_level_int = LOG_LEVEL_DEBUG;
            }
            else if (NULL != strcasestr(value, "error"))
            {
                cloudc_log_level_int = LOG_LEVEL_ERR;
            }
            else
            {
                cloudc_error("%s[%d]: parameter should only be debug or error, please double check", __func__, __LINE__);
            }
        }
#if 0
        if (NULL != (value = uci_lookup_option_string(cloudc_uci_ctx, cloudc_uci_sec, "register_flag")))
        {
            ap_register_flag = atoi(value);
            cloudc_debug("%s[%d]: ap_register_flag = %d", __func__, __LINE__, ap_register_flag);
        }
#endif

        if (NULL != (value = uci_lookup_option_string(cloudc_uci_ctx, cloudc_uci_sec, "cloud_server_url")))
        {
            cloud_server_url_new = strdup(value);
            strncpy(cloud_server_url, cloud_server_url_new, sizeof(cloud_server_url) - 1);
            cloudc_debug("%s[%d]: cloud_server_url = %s", __func__, __LINE__, cloud_server_url);
        }

        if (NULL != (value = uci_lookup_option_string(cloudc_uci_ctx, cloudc_uci_sec, "cloud_server_port")))
        {
            cloud_server_port = atoi(value);
            cloudc_debug("%s[%d]: cloud_server_port = %d", __func__, __LINE__, cloud_server_port);
        }
    }
    uci_unload(cloudc_uci_ctx, cloudc_uci_pkg); // 释放 pkg
    uci_free_context(cloudc_uci_ctx);
    return 0;
cleanup:
    uci_free_context(cloudc_uci_ctx);
    return -1;
}

void alljoyn_test()
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    cloudc_debug("%s[%d]: previous testcpp = %d", __func__, __LINE__, testcpp);
    test();
    cloudc_debug("%s[%d]: current testcpp = %d", __func__, __LINE__, testcpp);

	cloudc_debug("%s[%d]: gaojing dataconvert test ", __func__, __LINE__);
    //dataConvertTest();

	cloudc_debug("%s[%d]: gaojing prepare to 1st alljoynconfig ", __func__, __LINE__);
    StartAlljoynService();
    sleep(10);
    char *interfaces = "org.alljoyn.Config";
    char *objectPath = "/Config";
    int msgTypeGet = 0; //0:get; 1:set; 2:notify
    char *keyName = "DeviceName"; 
    char *keyValue = "This is GaoJing's device ^_^";
    configClientMain(interfaces,objectPath, msgTypeGet, keyName, NULL);
    
	cloudc_debug("%s[%d]: gaojing 1st alljoynconfig has finished", __func__, __LINE__);
	cloudc_debug("%s[%d]: gaojing prepare to 2nd alljoynconfig, please wait for 30s", __func__, __LINE__);  
    sleep(30);
    int msgTypeSet = 1; //0:get; 1:set; 2:notify
    configClientMain(interfaces,objectPath, msgTypeSet, keyName, keyValue);
    sleep(10);
    configClientMain(interfaces,objectPath, msgTypeGet, keyName, NULL);
	cloudc_debug("%s[%d]: gaojing 2nd alljoynconfig has finished", __func__, __LINE__);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
}

int main(int argc, char *argv[])
{
    //alljoyn_test();
    config_init();
    cloudc_ubus_init();
    
    socket_init();
    uloop_fd_add(&cloudc_monitor_uloop_fd, ULOOP_READ | ULOOP_BLOCKING); 

    cloudc_pthread_init();

    if (ap_check_register_condition())
    {
        ap_register();
    }
    else
    {
        cloudc_debug("%s[%d]: ap already registered, so there is no need to register again", __func__, __LINE__);
    }
    
    StartAlljoynService();
    uloop_run();
    ubus_free(cloudc_ctx);
    uloop_done();
    StopAlljoynService();

    return 0;
}


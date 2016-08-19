#include "share.h"
#include "parser.h" 
#include "cJSON.h" 

#define NEED_REGISTER 1
#define NO_NEED_REGISTER 0
#define CLOUDC_GET_CONFIG "/etc/config/cloudc"
#define CLOUD_SERVER_URL_LEN 128

/** public data **/
char ap_mac[MAX_MAC_LEN] = {0};
char ap_sn[MAX_SN_LEN] = {0};
char cloudc_server_ip[MAX_IP_LEN] = "115.29.49.52";
int cloudc_server_port = 80;
int ap_register_flag = 0;
/* attention: 
 * ap_register_flag need to set as 1 on officail release
 * now set it as 0 because it is easy to debug other func
 * */


/** function declare **/
int ap_check_register_condition(void);
int ap_register(void);
int config_init(int argc, char *argv[]);

int ap_check_register_condition(void)
{
    /* This func is used to check whether the ap can register to server, such as below:
     * if wan is up?
     * ...
     * currently, as the interface required, no matter the ap is registered or not before,
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
    //uloop_timeout_set(&register_recv_timer, RECV_TIME_OUT * 1000);

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);

    return -1;
}

int config_init(int argc, char *argv[])
{
    int i = 0;
    char tmpStr[32] = {0};
    char *tmpServerIp = NULL;
    char *tmpServerPort = NULL;
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);

    cloudc_debug("%s[%d]: argc = %d", __func__, __LINE__, argc);
    for(i = 0; i < argc; i ++)
    {
        cloudc_debug("%s[%d]: argv[%d] = %s\n", __func__, __LINE__, i, argv[i]);
        if(1 == i)
        {
            strncpy(tmpStr, argv[i], sizeof(tmpStr) - 1 );
            tmpServerIp = strtok(tmpStr,":");
            tmpServerPort = strtok(NULL, ":");

            if((NULL != tmpServerIp) && (NULL != tmpServerPort))
            {
                memset(cloudc_server_ip, 0, MAX_IP_LEN);
                strncpy(cloudc_server_ip, tmpServerIp, MAX_IP_LEN - 1);
                cloudc_server_port = atoi(tmpServerPort);
            }

            break;
        }
    }


    cloudc_debug("%s[%d]: cloudc_server_ip = %s, cloudc_server_port = %d", __func__, __LINE__, cloudc_server_ip, cloudc_server_port);

    get_ap_info(); //get ap_mac and ap_sn

    return 0;
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
}

int main(int argc, char *argv[])
{
    config_init(argc, argv);
    cloudc_ubus_init();

    socket_init();
    //uloop_fd_add(&cloudc_monitor_uloop_fd, ULOOP_READ | ULOOP_BLOCKING); 
    uloop_fd_add(&cloudc_monitor_uloop_fd, ULOOP_READ); 

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


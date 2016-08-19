#include "share.h"

int cloudc_ubus_init(void);
static int handle_loglevel_update(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method,
        struct blob_attr *msg);

void cloudc_detect_recv_register_status_timeout(struct uloop_timeout *t);

struct uloop_timeout register_recv_timer = {
    .cb = cloudc_detect_recv_register_status_timeout,
};

void cloudc_detect_send_timeout(struct uloop_timeout *t);

struct uloop_timeout send_timer = {
    .cb = cloudc_detect_send_timeout,
};

enum
{
    LOGLEVEL,
    __LOGLEVEL_MAX
};

struct ubus_context *cloudc_ctx;

static const struct blobmsg_policy handle_loglevel_policy[] = {
    [LOGLEVEL] = { .name = "loglevel", .type = BLOBMSG_TYPE_STRING },
};

static const struct ubus_method cloudc_object_methods[] = {
    //{.name = "disable", .handler = handle_cloudc_feature_down},
    UBUS_METHOD("loglevel", handle_loglevel_update, handle_loglevel_policy),
};

static struct ubus_object_type cloudc_object_type =
UBUS_OBJECT_TYPE("cloudc", cloudc_object_methods);

static struct ubus_object cloudc_object = {
    .name = "cloudc",
    .type = &cloudc_object_type,
    .methods = cloudc_object_methods,
    .n_methods = ARRAY_SIZE(cloudc_object_methods),
};

static int handle_loglevel_update(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method,
        struct blob_attr *msg)
{
    struct blob_attr *tb[__LOGLEVEL_MAX];
    char *log_buf = NULL;
    char cmd[128] = {0};

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    blobmsg_parse(handle_loglevel_policy, ARRAY_SIZE(handle_loglevel_policy), tb, blob_data(msg), blob_len(msg));

    if (tb[LOGLEVEL])
    {
        log_buf = blobmsg_data(tb[LOGLEVEL]);

        if ( NULL != strcasestr(log_buf, "debug"))
        {
            cloudc_log_level_int = LOG_LEVEL_DEBUG;
        }
        else if ( NULL != strcasestr(log_buf, "error"))
        {
            cloudc_log_level_int = LOG_LEVEL_ERR;
        }
        else
        {
            cloudc_error("%s[%d]: parameter should only be debug or error, please double check", __func__, __LINE__);
        }

        snprintf(cmd, sizeof(cmd), "uci set cloudc.global.loglevel=\"%s\"",log_buf);
        system(cmd);
        system("uci commit");
        cloudc_debug("%s[%d]: cmd = %s\n", __func__, __LINE__, cmd);

        cloudc_debug("%s[%d]: cloudc_log_level_int = %d", __func__, __LINE__, cloudc_log_level_int);
    }

    else
    {
        cloudc_debug("%s[%d]: wrong parameter,please double check", __func__, __LINE__);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;
}

int cloudc_ubus_init(void)
{
    int ret = -1;
    const char *ubus_socket = NULL;

    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    uloop_init();

    /* ubus init */
    cloudc_ctx = ubus_connect(ubus_socket);

    if (!cloudc_ctx) 
    {    
        cloudc_error(stderr, "Failed to connect to ubus");
        return -1;
    }    

    else 
    {    
        cloudc_debug("%s[%d]: connect to ubus succeed, cloudc_ctx = %p", __func__, __LINE__, cloudc_ctx);
    }    

    /* add connected fd into epoll fd set */
    ubus_add_uloop(cloudc_ctx); 

    /* add ubus object */
    ret = ubus_add_object(cloudc_ctx, &cloudc_object);

    if (ret)
    {    

        cloudc_error(stderr, "Failed to add object: %s", ubus_strerror(ret));
    }    
    else 
    {    
        cloudc_debug("%s[%d]: ubus add object successfully, ret = %d", __func__, __LINE__, ret);
    }    

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return 0;

}

void cloudc_detect_recv_register_status_timeout(struct uloop_timeout *t)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    if ((0 == ap_register_flag) || (2 == ap_register_flag))
    {
        /* register failed, need to send register request again */
        /* can enter mainloop */
        cloudc_debug("%s[%d]: register succeed or alread registered, ap_register_flag = %d ", __func__, __LINE__, ap_register_flag);
        ap_register_flag = 1;
    }
    else
    {
        /* register failed, need to send register request again */
        cloudc_debug("%s[%d]: register failed, need to register again, ap_register_flag = %d ", __func__, __LINE__, ap_register_flag);
        ap_register();
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
}

void cloudc_detect_send_timeout(struct uloop_timeout *t)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);
    close(cloudc_monitor_uloop_fd.fd);
    socket_create();
    socket_connect();
    //ap_register();
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
}

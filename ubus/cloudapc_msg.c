#include <stdio.h>
#include <unistd.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include "cloudapc_msg.h"

static struct ubus_context *ctx;
static struct ubus_event_handler listener;

//监听事件触发回调（在这里接收通信的信息）
static void event_receive_cb(struct ubus_context *ctx, struct ubus_event_handler *ev, const char *type, struct blob_attr *msg)
{
    printf("This is a event receive cb, you can do something ...\n");
#if 0 
   enum {
        EV_ACTION,
        EV_IFNAME,
        __EV_MAX
    };
    static const struct blobmsg_policy ev_policy[__EV_MAX] = {
        [EV_ACTION] = { .name = "action", .type = BLOBMSG_TYPE_STRING },
        [EV_IFNAME] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
    };
    struct blob_attr *tb[__EV_MAX];

    blobmsg_parse(ev_policy, __EV_MAX, tb, blob_data(msg), blob_len(msg));

    /* do something */
    char *opt = blobmsg_get_string(tb[INPUT]);
    strcpy(opt_buf,opt);
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "input1",opt_buf);
    blobmsg_add_string(&b, "input2",opt_buf);
    ubus_send_reply(ctx, req, b.head);
#endif
}

//注册监听事件
void cloudapc_ubus_listen_event(void)
{
    char *event = "network.interface";
    ubus_register_event_handler(ctx, &listener, event);//可以注册多个监听事件
    ubus_add_uloop(ctx);
}

//事件监听初始化
void cloudapc_ubus_event_handler_init(void)
{
    memset(&listener, 0, sizeof(listener));
    listener.cb = event_receive_cb;
}

//ubus使用初始化
int cloudapc_ubus_init(void)
{
    const char *ubus_socket = NULL;
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1; 
    }   
    
    return 0;
}

//退出ubus
void cloudapc_ubus_exit(void)
{
    ubus_free(ctx);
}

int main()
{
    uloop_init();
    cloudapc_ubus_init();
    cloudapc_ubus_event_handler_init();

    cloudapc_ubus_listen_event();

    uloop_run();

    cloudapc_ubus_exit();
    uloop_done();
    return 0;
}

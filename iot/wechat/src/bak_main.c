#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>     /* for AF_NETLINK */
#include <sys/ioctl.h>      /* for ioctl */
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/netlink.h>  /* for sockaddr_nl */
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <libubox/ustream.h>
#include <libubus.h>
#include <signal.h>
#include <libubox/blobmsg_json.h>
#include <uci.h>
#include "wechat_log.h"
#include "WeChatAPI_C.h"

extern int wechat_log_level_int;
const char *dev_license = "weixinzhengshu";

onReceiveResponse onWeChatCallBack;
onReceiveNotify onWeChatReceiveNotify;
onSDKEventCallback onWeChatHandleEvent;

void printWeChatSDKInfo(void);
#if 0
void onWeChatCallBack(int taskid, int errcode, unsigned int funcid, const unsigned char *body, unsigned int bodylen);
void onWeChatReceiveNotify(unsigned int funcid, const unsigned char *_body, unsigned int _bodylen);
void onWeChatHandleEvent(enum EventValue event_value);

void onWeChatCallBack(int taskid, int errcode, unsigned int funcid, const unsigned char *body, unsigned int bodylen)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);

    wechat_debug("%s[%d]: receive wechat callback", __func__, __LINE__);
    wechat_debug("%s[%d]: taskdid = %d, errcode = %d, funcid = %d, body = %s, bodylen = %d", __func__, __LINE__, taskid, errcode, funcid, body, bodylen);

    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
}

void onWeChatReceiveNotify(unsigned int funcid, const unsigned char *_body, unsigned int _bodylen)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);

    wechat_debug("%s[%d]: receive wechat onwReceiveNotify", __func__, __LINE__);
    wechat_debug("%s[%d]: funcid = %d, _body = %s, _bodylen = %d", __func__, __LINE__, funcid, _body, _bodylen);

    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
}

void onWeChatHandleEvent(enum EventValue event_value)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);
    wechat_debug("%s[%d]: receive wechat handle event", __func__, __LINE__);

    switch(event_value)
    {
        case EVENT_VALUE_LOGIN:
            wechat_debug("%s[%d]: device login", __func__, __LINE__);
            break;

        case EVENT_VALUE_LOGOUT:
            wechat_debug("%s[%d]: device logout", __func__, __LINE__);
            break;

        default:
            wechat_debug("%s[%d]: unknown event", __func__, __LINE__);
    }

    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
}
#endif

void printWeChatSDKInfo(void)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);

    wechat_error("%s[%d]: SDK version: %s", __func__, __LINE__, WeChatAPI_getSDKVersion());
    wechat_error("%s[%d]: Vender ID: %s", __func__, __LINE__, WeChatAPI_getVenderId());
    wechat_error("%s[%d]: Device ID: %s", __func__, __LINE__, WeChatAPI_getDeviceId());

    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
}

int main(void)
{
    int a = print();
    printf("a = %d, device_license = %s\n", a, dev_license);

    if(false == WeChatAPI_start(dev_license, strlen(dev_license))) //初始化启动SDK
    {
        wechat_debug("%s[%d]: failed to start SDK, please check if your dev_license:%s is right", __func__, __LINE__, dev_license);
        //return -1;
    }

    WeChatAPI_setCallBack(onWeChatCallBack); //注册任务执行回调接口,所有的task都是通过本接口返回
    WeChatAPI_setNotifyCallBack(onWeChatReceiveNotify); //服务器推送消息回调接口
    WeChatAPI_setSDKEventCallBack(onWeChatHandleEvent); //SDK事件通知回调函数

    printWeChatSDKInfo(); //打印设备信息

    while(1)
    {
        printf("hello world\n");
        sleep(15);
    }

    return 0;
}

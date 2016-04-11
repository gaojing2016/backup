#include "wechat_log.h"
#include <pthread.h>

int WeChatCallBack_pthreadInit(void);
void *process_onWeChatCallBack(void *arg);
void *process_onWeChatReceiveNotify(void *arg);
void *process_onWeChatHandleEvent(void *arg);

#if 0
WeChatAPI_setCallBack(onWeChatCallBack); //注册任务执行回调接口,所有的task都是通过本接口返回
WeChatAPI_setNotifyCallBack(onWeChatReceiveNotify); //服务器推送消息回调接口
WeChatAPI_setSDKEventCallBack(onWeChatHandleEvent); //SDK事件通知回调函数
#endif

void *process_onWeChatCallBack(void *arg)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);
    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
    return ((void *)0);
}

void *process_onWeChatReceiveNotify(void *arg)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);
    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
    return ((void *)0);
}

void *process_onWeChatHandleEvent(void *arg)
{
    wechat_debug("%s[%d]: Enter", __func__, __LINE__);
    wechat_debug("%s[%d]: Exit", __func__, __LINE__);
    return ((void *)0);
}

int WeChatCallBack_pthreadInit(void)
{
    pthread_t wechat_tid;
    int ret1 = -1;
    int ret2 = -1;
    int ret3 = -1;

    ret1 = pthread_create(&wechat_tid, NULL, process_onWeChatCallBack, NULL);

    if(ret1 != 0)
    {
        perror("can't create thread: %s\n");
    }
    else
    {
        wechat_debug("%s[%d]: ret1 = %d", __func__, __LINE__, ret1);
    }

    ret2 = pthread_create(&wechat_tid, NULL, process_onWeChatReceiveNotify, NULL);
    ret3 = pthread_create(&wechat_tid, NULL, process_onWeChatHandleEvent, NULL);

    sleep(1);

    return 0;
}


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>     /* for AF_NETLINK */
#include <sys/ioctl.h>      /* for ioctl */
#include <linux/netlink.h>  /* for sockaddr_nl */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <libubus.h>
#include <signal.h>
#include <libubox/ustream.h>
#include <libubox/blobmsg_json.h>


static struct ubus_context *adapter_ctx;

int ubusInit(void)
{
    int ret = -1; 
    const char *ubus_socket = NULL;

    uloop_init();
    adapter_ctx = ubus_connect(ubus_socket); //ubus使用初始化

    if (!adapter_ctx)
    {   
        perror("Failed to connect to ubus: ");
        return -1; 
    }   

    else
    {   
        printf("connect to ubus succeed, adapter_ctx = %p", adapter_ctx);
    }   

    ubus_add_uloop(adapter_ctx); //Add the connected fd into epoll fd set

    return 0;
}



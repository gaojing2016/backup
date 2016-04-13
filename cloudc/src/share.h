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
#include "cloudc_log.h"

#define CLOUD_SERVER_MANAGE_IP "192.168.3.19"
#define CLOUD_SERVER_MANAGE_PORT 3000
#define RECV_MAX_BUF_LEN 4096
#define SEND_MAX_BUF_LEN 4096
#define MAX_IPK_NAME_LEN 20
#define RECV_TIME_OUT 30
#define SEND_TIME_OUT 30

extern struct uloop_fd cloudc_monitor_uloop_fd;
extern struct sockaddr_in server_addr;
extern struct ubus_context *cloudc_ctx;

extern char ap_mac[18] ;
extern char ap_sn[40] ;
extern int cloudc_log_level_int;
extern int ap_register_flag;
extern struct list_node queue_head;
extern struct uloop_timeout register_recv_timer;
extern struct uloop_timeout send_timer;
extern pthread_mutex_t mutex;

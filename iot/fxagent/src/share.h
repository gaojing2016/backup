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
#if 0
#define CLOUD_SERVER_MANAGE_IP "192.168.1.10"
#define CLOUD_SERVER_MANAGE_PORT 3000
#define CLOUD_SERVER_MANAGE_IP "115.29.49.52" 
#define CLOUD_SERVER_MANAGE_PORT 80 
#endif
#define RECV_MAX_BUF_LEN 4096
#define SEND_MAX_BUF_LEN 4096
#define RECV_TIME_OUT 30
#define SEND_TIME_OUT 30

#define MAX_INTERFACE_LEN 128
#define MAX_OBJECTPATH_LEN 128
#define MAX_DEVICETYPE_LEN 32
#define MAX_DEVICEID_LEN 32
#define MAX_DEVICESN_LEN 32
#define MAX_ONLINESTATUS_LEN 4
#define MAX_MANUFACTURE_LEN 32
#define MAX_MODULENUMBER_LEN 32
#define MAX_USERID_LEN 32


#define MAX_DEVDATA_LEN 512
#define MAX_MAC_LEN 18
#define MAX_SN_LEN 32
#define MAX_IP_LEN 16

#define DATA_CONVERT_FILE_PATH "/etc/config/DataConvert.xml"
#define DEVICE_DATABASE_PATH "/usr/bin/device.db"

extern struct uloop_fd cloudc_monitor_uloop_fd;
extern struct sockaddr_in server_addr;
extern struct ubus_context *cloudc_ctx;

extern char ap_mac[MAX_MAC_LEN] ;
extern char ap_sn[MAX_SN_LEN] ;
extern char cloudc_server_ip[MAX_IP_LEN];
extern int cloudc_server_port; 
extern int cloudc_log_level_int;
extern int ap_register_flag;
extern struct list_node queue_head;
extern struct uloop_timeout register_recv_timer;
extern struct uloop_timeout send_timer;
extern pthread_mutex_t mutex;

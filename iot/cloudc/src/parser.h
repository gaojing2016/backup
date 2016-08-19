/* type value */
#define CMD_OPKG_UPDATE "opkg"
#define CMD_RSP_REGISTER "rsp_gateway_online"
#define CMD_INSTALL_IPK "install"
#define CMD_UNINSTALL_IPK "uninstall"
#define CMD_UPGRADE_IPK "upgrade"
#define CMD_GET_INSTALL_IPK "query_installed"
#define CMD_GET_RUNNING_IPK "query_running"

#define CMD_START_IPK_SERVICE "start" 
#define CMD_STOP_IPK_SERVICE "stop"
#define CMD_ENABLE_IPK_SERVICE "enable"
#define CMD_DISABLE_IPK_SERVICE "disable"

#define CMD_ALLJOYN_GET_OPERATION "get"
#define CMD_ALLJOYN_SET_OPERATION "set"

#define NOT_EMPTY 0
#define EMPTY -1
#define MAX_RPC_CMD_LEN 32
#define MAX_OPKG_UPDATE_URL_LEN 128

#define MAX_USER_ID_LEN 32
#define MAX_DEVICE_ID_LEN 64
#define MAX_DEVICE_TYPE_LEN 32

#define MAX_IPK_NAME_LEN 32
#define MAX_NODE_VALUE_LEN 32

typedef enum
{
    eRegister = 1,
    eIpkOperation,
    eQueryOperation,
    eOpkgOperation,
    eServiceOperation,
    eAlljoynGetOperation,
    eAlljoynSetOperation,
    eInvalidParam
}eOpType;

typedef struct ipk_info
{
    char op_ipk_name[MAX_IPK_NAME_LEN];
    char node_name_convert[MAX_IPK_NAME_LEN];
    char node_value[MAX_NODE_VALUE_LEN];
    int status;
    struct ipk_info *next;
}ipk_info;

typedef struct http_value
{
    int     rpc_flag;
    int     register_status;
    int     serial_num;
    int     opkg_update_flag;
    int     real_ipk_num;
    char    rpc_cmd[MAX_RPC_CMD_LEN];
    char    opkg_update_url[MAX_OPKG_UPDATE_URL_LEN];
    struct ipk_info *ipk_name_head;
    char user_id[MAX_USER_ID_LEN];
    char device_id[MAX_DEVICE_ID_LEN];
    char device_type[MAX_DEVICE_TYPE_LEN];  //temp add for alljoyn
    /*char    returncode[8];//200 OK or 500 */
}http_value;

/* 定义队列的结构 */
struct list_node
{
    struct list_node *pre;
    struct list_node *next;
}list_node;

/* 定义队列节点的结构 */
struct task_data_node
{
    struct list_node task_list_node;
    struct http_value data;
}task_data_node;


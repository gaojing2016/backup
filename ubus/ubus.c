
libubus
数据结构

struct ubus_event_handler 
{
    struct ubus_object obj;
    ubus_event_handler_t cb;
};

struct ubus_context 
{
    struct list_head requests;
    struct avl_tree objects;    /** client端object链表头 */
    struct list_head pending;

    struct uloop_fd sock;       /** client端sock对象 */

    uint32_t local_id;          /** ubusd分配的client id */
    uint16_t request_seq;
    int stack_depth;

    /** 断开连接回调函数 */
    void (*connection_lost)(struct ubus_context *ctx);

    struct 
    {
        struct ubus_msghdr hdr;
        char data[UBUS_MAX_MSGLEN];
    } msgbuf;                   /** 通信报文 */
};

struct ubus_object_data 
{
    uint32_t id;
    uint32_t type_id;
    const char *path;
    struct blob_attr *signature;
};

struct ubus_request_data 
{
    uint32_t object;
    uint32_t peer;
    uint16_t seq;

    /* internal use */
    bool deferred;
    int fd;
};

struct ubus_request 
{
    struct list_head list;

    struct list_head pending;
    int status_code;
    bool status_msg;
    bool blocked;
    bool cancelled;
    bool notify;

    uint32_t peer;
    uint16_t seq;

    ubus_data_handler_t raw_data_cb;
    ubus_data_handler_t data_cb;
    ubus_fd_handler_t fd_cb;
    ubus_complete_handler_t complete_cb;

    struct ubus_context *ctx;
    void *priv;
};

struct ubus_notify_request 
{
    struct ubus_request req;

    ubus_notify_complete_handler_t status_cb;
    ubus_notify_complete_handler_t complete_cb;

    uint32_t pending;
    uint32_t id[UBUS_MAX_NOTIFY_PEERS + 1];
};

struct ubus_auto_conn 
{
    struct ubus_context ctx;
    struct uloop_timeout timer;
    const char *path;
    ubus_connect_handler_t cb;
};

=============接口说明=================

* 初始化client端context结构，并连接ubusd
struct ubus_context *ubus_connect(const char *path) 

* 与ubus_connect()函数基本功能相同，但此函数在连接断开后会自动进行重连
void ubus_auto_connect(struct ubus_auto_conn *conn)

* 注册新事件
int ubus_register_event_handler(struct ubus_context *ctx,
        struct ubus_event_handler *ev,
        const char *pattern)

* 发出事件消息
int ubus_send_event(struct ubus_context *ctx, const char *id,
        struct blob_attr *data)

* 向ubusd查询指定UBUS_ATTR_OBJPATH对应对象信息内容
* 内容通过输入回调函数ubus_lookup_handler_t由调用者自行处理
int ubus_lookup(struct ubus_context *ctx, const char *path,
        ubus_lookup_handler_t cb, void *priv)

* 向ubusd查询指定UBUS_ATTR_OBJPATH对应的ID号
int ubus_lookup_id(struct ubus_context *ctx, const char *path, uint32_t *id)


libubus-io
接口说明

* libubus关注的报文中数据属性列表
static const struct blob_attr_info ubus_policy[UBUS_ATTR_MAX] = 
{
    [UBUS_ATTR_STATUS] = { .type = BLOB_ATTR_INT32 },
    [UBUS_ATTR_OBJID] = { .type = BLOB_ATTR_INT32 },
    [UBUS_ATTR_OBJPATH] = { .type = BLOB_ATTR_STRING },
    [UBUS_ATTR_METHOD] = { .type = BLOB_ATTR_STRING },
    [UBUS_ATTR_ACTIVE] = { .type = BLOB_ATTR_INT8 },
    [UBUS_ATTR_NO_REPLY] = { .type = BLOB_ATTR_INT8 },
    [UBUS_ATTR_SUBSCRIBERS] = { .type = BLOB_ATTR_NESTED },
};


* 把libubus关注的无序报文转化为有序的blob_attr数组
__hidden struct blob_attr **ubus_parse_msg(struct blob_attr *msg)

     * 发送报文
     * 
     * @param ctx    - client上下文对象
     * @param seq    - 报文顺序号 hdr.seq
     * @param msg    - 报文内容 
     * @param cmd    - 报文类型 hdr.type
     * @param perr   -
     * @param fd     - 需传递给对端的描述符，等于-1表示不需传递
int __hidden ubus_send_msg(struct ubus_context *ctx, uint32_t seq,
        struct blob_attr *msg, int cmd, uint32_t peer, int fd)

* client端fd收包处理函数
void __hidden ubus_handle_data(struct uloop_fd *u, unsigned int events)

* client端轮询fd收包处理函数
void __hidden ubus_poll_data(struct ubus_context *ctx, int timeout)

* client连接ubusd server
int ubus_reconnect(struct ubus_context *ctx, const char *path)


libubus-obj
数据结构

struct ubus_object 
{
    struct avl_node avl;  /** 关系到struct ubus_context的objects */

    const char *name;     /** UBUS_ATTR_OBJPATH */
    uint32_t id;          /** 由ubusd server分配的obj id */

    const char *path;
    struct ubus_object_type *type;

    /** 第1次被订阅或最后1次补退订时被调用 */
    ubus_state_handler_t subscribe_cb;
    bool has_subscribers;    /** 此对象是否被订阅 */

    const struct ubus_method *methods;  /** 方法数组 */
    int n_methods;                      /** 方法数组个数 */
};

struct ubus_object_type 
{
    const char *name;
    uint32_t id;            /** 由ubusd server分配的obj type id */

    const struct ubus_method *methods;  /** 方法数组 */
    int n_methods;                      /** 方法数组个数 */
};

struct ubus_method 
{
    const char *name;         /** 方法名称 */
    ubus_handler_t handler;   /** 方法处理回调函数 */

    unsigned long mask;                   /** 参数过滤掩码 */
    const struct blobmsg_policy *policy;  /** 参数过滤列表 */
    int n_policy;                         /** 参数过滤列表个数 */
};


接口说明

/**
 * client端向ubusd server请求增加一个新object
 */
int ubus_add_object(struct ubus_context *ctx, struct ubus_object *obj)

/**
* client端向ubusd server请求删除一个object
*/
int ubus_remove_object(struct ubus_context *ctx, struct ubus_object *obj)

/**
* 处理收到与object相关报文
*/
void __hidden ubus_process_obj_msg(struct ubus_context *ctx, 
        struct ubus_msghdr *hdr)

/**
* 处理UBUS_MSG_INVOKE报文
*/
static void
ubus_process_invoke(struct ubus_context *ctx, struct ubus_msghdr *hdr,
        struct ubus_object *obj, struct blob_attr **attrbuf)

/**
* 处理UBUS_MSG_UNSUBSCRIBE报文
*/
static void
ubus_process_unsubscribe(struct ubus_context *ctx, struct ubus_msghdr *hdr,
        struct ubus_object *obj, struct blob_attr **attrbuf)

/**
* 处理UBUS_MSG_NOTIFY报文
*/
static void
ubus_process_notify(struct ubus_context *ctx, struct ubus_msghdr *hdr,
        struct ubus_object *obj, struct blob_attr **attrbuf)


libubus-sub
数据结构

struct ubus_subscriber 
{
    struct ubus_object obj;

    ubus_handler_t cb;
    ubus_remove_handler_t remove_cb;
};


接口说明

/**
 * 
 */
int ubus_register_subscriber(struct ubus_context *ctx, struct ubus_subscriber *s)

/**
* 
*/
int ubus_subscribe(struct ubus_context *ctx, struct ubus_subscriber *obj, 
        uint32_t id)

/**
*
*/
int ubus_unsubscribe(struct ubus_context *ctx, struct ubus_subscriber *obj,
        uint32_t id)


libubus-req
数据结构

struct ubus_request_data 
{
    uint32_t object;
    uint32_t peer;
    uint16_t seq;

    /* internal use */
    bool deferred;
    int fd;
};

struct ubus_request 
{
    struct list_head list;

    struct list_head pending;
    int status_code;
    bool status_msg;
    bool blocked;
    bool cancelled;
    bool notify;

    uint32_t peer;
    uint16_t seq;

    ubus_data_handler_t raw_data_cb;
    ubus_data_handler_t data_cb;
    ubus_fd_handler_t fd_cb;
    ubus_complete_handler_t complete_cb;

    struct ubus_context *ctx;
    void *priv;
};

struct ubus_notify_request 
{
    struct ubus_request req;

    ubus_notify_complete_handler_t status_cb;
    ubus_notify_complete_handler_t complete_cb;

    uint32_t pending;
    uint32_t id[UBUS_MAX_NOTIFY_PEERS + 1];
};


接口说明

/**
 * 发送回应信息，消息类型UBUS_MSG_DATA
 */
int ubus_send_reply(struct ubus_context *ctx, struct ubus_request_data *req,
        struct blob_attr *msg)

/**
* 异步通知指定object执行其方法
*/
int ubus_invoke_async(struct ubus_context *ctx, uint32_t obj, const char *method,
        struct blob_attr *msg, struct ubus_request *req)

/**
* 同步通知指定object执行其方法
*/

int ubus_invoke(struct ubus_context *ctx, uint32_t obj, const char *method,
        struct blob_attr *msg, ubus_data_handler_t cb, void *priv,
        int timeout)

/**
* 异步发出通知消息
*/
int ubus_notify_async(struct ubus_context *ctx, struct ubus_object *obj,
        const char *type, struct blob_attr *msg,
        struct ubus_notify_request *req)

/**
 * 同步发出通知消息
*/
int ubus_notify(struct ubus_context *ctx, struct ubus_object *obj,
        const char *type, struct blob_attr *msg, int timeout)


例子
向ubusd注册新object

定义object方法:

enum 
{
    OBJ_SET_ARG1,
    OBJ_SET_ARG2,
    __OBJ_SET_ATTR_MAX
};

/** 定义set方法参数列表 */
static const struct blobmsg_policy obj_set_attrs[__OBJ_SET_ATTR_MAX] = {
    [OBJ_SET_ARG1] = { .name = "arg1", .type = BLOBMSG_TYPE_STRING },
    [OBJ_SET_ARG2 ] = { .name = "arg2", .type = BLOBMSG_TYPE_STRING },
};

static struct ubus_method obj_methods[] = {
    { .name = "enable", .handler = obj_enable },
    UBUS_METHOD("set", obj_set, obj_set_attrs),
    { .name = "dump", .handler = obj_dump },
};


定义object类型:

static struct ubus_object_type obj_type = 
UBUS_OBJECT_TYPE("my_obj", obj_methods);

定义object:

static struct ubus_object obj = {
    .name = "myobj",
    .type = &obj_type,
    .methods = obj_methods,
    .n_methods = ARRAR_SIZE(obj_methods),
};


注册新object:

uloop_init();
struct ubus_context *ubus_ctx = ubus_connect(NULL);
ubus_add_uloop(ubus_ctx);
ubus_add_object(ubus_ctx, &obj);
uloop_run();

向ubusd注册事件监听
定义事件触发回调方法:

static void
event_receive_cb(struct ubus_context *ctx, struct ubus_event_handler *ev, 
        const char *type, struct blob_attr *msg)
{
    enum 
    {
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
}

注册监听事件

static void
event_listen(void)
{
    static struct ubus_event_handler listener;
    memset(&listener, 0, sizeof(listener));

    /** 监听netwrok.interface事件 */
    ubus_register_event_handler(ubus_ctx, &listener, "network.interface");
}

向ubusd发送命令

定义命令返回回调方法:

static void
command_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    if (!msg)
        return;

    enum 
    {
        ADDR_IPV4,
        __ADDR_MAX,
    };
    static const struct blobmsg_policy policy[__ADDR_MAX] = {
        [ADDR_IPV4] = { .name = "ipv4-address", .type = BLOBMSG_TYPE_ARRAY },
    };
    struct blob_attr *tb[__ADDR_MAX];

    blobmsg_parse(policy, __ADDR_MAX, tb, blobmsg_data(msg), blobmsg_len(msg));

    /** do something */
}

发送命令:

static void
invoke_command(char *net)
{
    uint32_t id;
    char path[64] = {0};

    sprintf(path, "network.interface.%s", net);

    /** lookup `network.interface.%s` object id */
    ubus_lookup_id(ubus_ctx, path, &id);

    /** invoke command `status` */
    ubus_invoke(ubus_ctx, id, "status", NULL, command_cb, NULL, 500);
}





**********************************************************************************************************************************************
//Object and Object paths
Object paths are binding can name object instance, and allow applications to refer to them.
In OpenWRT, the object path is namespace like network,interface.lan

**********************************************************************************************************************************************
//Methods and Notifications
Methods are operations that can be invoked on an object, which optional input parameters and output.
Notifications are broadcasts from the object to any interested observers of the object.
The notifications may contain a data payload.

**********************************************************************************************************************************************
//Calling a method
A method call in ubus consists of two messages:
A call message from process A to process B and they reply message from process B to process A.
The send message and reply message are both routed through the ubus daemon.
The call mesage contains the method arguments.
The reply messages may be error messages, or may contain method returned data.


//Call Process
1.The call method messages contains the ubus connection context, the destination object id, the method name, the method arguments.
2. The method call message is send to the ubus daemon
3. The ubus daemon lookup the destination object id, if a process owns the object instance,
    then the daemon will forward the method call to the find process.
    Otherwise the ubus daemon creates an error messages and sends the error message back to the message call as reply.
    4. The receiving process will parse the ubus object messages, and find the call method and arguments belong to the method.
    Then match the object methods in object instance, if find matched method, will invoke the method and then send the reply messages.
    5. Ubus daemon receive the reply message and forward the reply message to the process that made the method call.
    6. The reply messages is transferred as ubus blob messages structure which is TLV (Type-Length-Value) based binary messages type.
    7. The process received the reply message should parse the message and format to human-nice message type as JSON or XML.


    **********************************************************************************************************************************************
    //Notify Notification

    A notification in ubus consists of a single messages, send by one process to any number of other processes, 
    which means the notification is a unidirectional broadcast, no need expected reply message.

    The notification sender do not know the notifications recipients, 
    it just send the notification onto bus The interest recipients should subscribe the sender object with the bus daemon.


    **********************************************************************************************************************************************
    //Notification Process

    1. Add notification object onto ubus daemon
    2. The notification message contains below info:
    ubus connection context, the notification sender object ID, the notification type and optional arguments with the type.
    3. Any process on the ubus can subscribe the notification object.
    The bus may has a list of subscribers, which will match the observers when daemon handle the notification message.
    4. The ubus daemon check the notification and determines which processes are interested in it.
    Then send the notification to all of the interested processes.
    5.Each subscriber process receiving the notification decides what to do with the notification message.


    **********************************************************************************************************************************************
    //Blob_buf structure on ubus
    Blob_attr
    Blob_msg
    Blob_buf


    **********************************************************************************************************************************************
    //How to use ubus
    //Server Main process(##########demo1##########)
    1. Define a object with some abstract methods
    2. Connect the server process to ubus daemon and get a ubus_context, 
    the context will contained the connected fd, registered fd callback and an AVL tree to manage all objects information with this connection
    3. Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set
    4. Add the defined object into ubusd
    5. Forever loop to epoll the fd set


    //What to do in method handler(##########demo1##########)
    1. Parse the blob_attr msg into a blob_attr table, which can easy using by index the table by msg ID
    2. Get the method arguments according to msg id, the handler maybe call method in another objects or invoke a shell script to do some service, etc
    3. Prepare the response msg into blob_buff and send the response to ubus daemon, 
    which will forward the response to request client if not specify “no_reply” or ”deferred” flag
    4. If specify “deferred” flag in req context in the method handler, 
    which means the server process will not expect the response in this request handler and just complete this request.


    //Client Main Process(##########demo2##########)
    1. Connect the client process to ubus daemon,  
    will get the ubus context, the context will contained the connected fd, 
    registered fd callback and an AVL tree to manage all objects information with this connection
    2. Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set 
    3. Look up the target object id by the object path in ubus context
    4. Arrange the ubus call method and method arguments into blob_buff.
    5. Invoke ubus high level API to invoke a method on a specific object,   and wait for the reply .


    **********************************************************************************************************************************************
    //How to use notification 
    //Subscriber(##########demo3##########)
    1. Connect the process to ubus daemon,  
    will get the ubus context, the context will contained the connected fd, 
    registered fd callback and an AVL tree to manage all objects information with this connection
    2. Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set
    3. Define a subscriber object, which contain a ubus object and  a callback to handle received subscribe notification
    4. Add ubus object onto ubus daemon
    5. Specify callback handler to handle notification
6. Subscribe interested object(notify object)


    //Notification Sender(##########demo4##########)
    1. Connect the process to ubus daemon,  
    will get the ubus context, the context will contained the connected fd, 
    registered fd callback and an AVL tree to manage all objects information with this connection
    2. Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set 
    3. Define a notify object
    4. Add notify object onto bus 
    5. Prepare notify type and arguments when actually an event happens 
    6. Broadcast the event notification to bus 
    **********************************************************************************************************************************************


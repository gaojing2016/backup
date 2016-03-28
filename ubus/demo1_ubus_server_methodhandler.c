/*How to use ubus
Server Main process
M1.  Define a object with some abstract methods
M2.  Connect the server process to ubus daemon and get a ubus_context, 
     the context will contained the connected fd, registered fd callback and an AVL tree to manage all objects information with this connection
M3.  Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set
M4.  Add the defined object into ubusd
M5.  Forever loop to epoll the fd set

What to do in method handler
H1.   Parse the blob_attr msg into a blob_attr table, which can easy using by index the table by msg ID
H2.  Get the method arguments according to msg id, the handler maybe call method in another objects or invoke a shell script to do some service, etc
H3.  Prepare the response msg into blob_buff and send the response to ubus daemon, 
     which will forward the response to request client if not specify “no_reply” or ”deferred” flag
H4.  If specify “deferred” flag in req context in the method handler, 
     which means the server process will not expect the response in this request handler and just complete this request.
*/

#include <libubox/blobmsg_json.h>
#include "libubus.h"

    static struct ubus_context *ctx;
static int test_hello(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method,
        struct blob_attr *msg)
{
    struct hello_request *hreq;
    struct blob_attr *tb[__HELLO_MAX];
    const char *format = "%s received a message: %s";
    const char *msgstr = "(unknown)";


    // H1. Parse the blob_attr msg(blob_data(msg)) into a blob_attr
    //table (tb), which can easily use by msg ID to index the table
    blobmsg_parse(hello_policy, ARRAY_SIZE(hello_policy), tb, blob_data(msg), blob_len(msg));

    // H2.  Get method arguments by msg ID
    if (tb[HELLO_MSG])
        msgstr = blobmsg_data(tb[HELLO_MSG]);

    hreq = calloc(1, sizeof(*hreq) + strlen(format) + strlen(obj->name) + strlen(msgstr) + 1);
    sprintf(hreq->data, format, obj->name, msgstr);
    // H4. Defer the reply for the request
    // The reply will be making in timer callback 
    ubus_defer_request(ctx, req, &hreq->req);
    hreq->timeout.cb = test_hello_reply;
    uloop_timeout_set(&hreq->timeout, 1000);
    return 0;
}

// Define hello method with test_hello handle 
//hello policy tell ubusd  the object method parameters type
static const struct ubus_method test_methods[] = { 
    UBUS_METHOD("hello", test_hello, hello_policy),
    };

// M1. Define test_object
static struct ubus_object test_object = {
    .name = "test",
    .type = &test_object_type,
    .methods = test_methods,
    .n_methods = ARRAY_SIZE(test_methods),
};

static void server_main(void)
{
    int ret;
    // M4.  Add the defined object into ubusd 
    ret = ubus_add_object(ctx, &test_object);

    if (ret)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    }
    // M5.  Forever loop to epoll the fd set and handle the available fd 
    uloop_run();
}

int main(int argc, char **argv)
{
    const char *ubus_socket = NULL;
    int ch;
    uloop_init();
    signal(SIGPIPE, SIG_IGN);
    // M2.  Connect to ubusd, will get the ubus_context
    ctx = ubus_connect(ubus_socket);

    if (!ctx) 
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }
    // M3.  Add the ubus connection into epoll set 
    ubus_add_uloop(ctx);
    server_main();
    ubus_free(ctx);
    uloop_done();
    return 0;
}


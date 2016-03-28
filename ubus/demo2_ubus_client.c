/*Client Main Process
M1.  Connect the client process to ubus daemon,  will get the ubus context, 
     the context will contained the connected fd, registered fd callback and an AVL tree to manage all objects information with this connection
M2.  Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set
M3.  Look up the target object id by the object path in ubus context
M4.  Arrange the ubus call method and method arguments into blob_buff.
M5.  Invoke ubus high level API to invoke a method on a specific object,   and wait for the reply .
*/

/* invoke a method on a specific object */
int ubus_invoke(struct ubus_context *ctx, uint32_t obj, const char *method, struct blob_attr *msg, ubus_data_handler_t cb, void *priv,
        int timeout);
//Specify a callback to handle the response blob_msg to human-nice message format like JSON or XML
//Or
//M4. For some case, we may not need to wait for the response, should call asynchronous version invoke
/* asynchronous version of ubus_invoke() */
int ubus_invoke_async(struct ubus_context *ctx, uint32_t obj, const char *method, struct blob_attr *msg, struct ubus_request *req);

static int ubus_cli_call(struct ubus_context *ctx, int argc, char **argv)
{
    uint32_t id;
    int ret;

    if (argc < 2 || argc > 3)
    {
        return -2;
    }

    //M4. Arrange the ubus call method and method arguments into blob_buff
    blob_buf_init(&b, 0);

    if (argc == 3 && !blobmsg_add_json_from_string(&b, argv[2])) 
    {
        if (!simple_output)
        {
            fprintf(stderr, "Failed to parse message data\n");
        }

        return -1;
    }

    //M3. Look up the target object id by the object path
    ret = ubus_lookup_id(ctx, argv[0], &id);
    
    if (ret)
    {
        return ret;
    }
    //M5. Invoke the method and wait for the reply
    // receive_call_result_data callback will convert blob_attr data to JSON format
    return ubus_invoke(ctx, id, argv[1], b.head, receive_call_result_data,      NULL, timeout *       1000);
}

int main(int argc, char **argv)
{
    const char *ubus_socket = NULL;
    int ch;

    while ((ch = getopt(argc, argv, "cs:")) != -1) 
    {
        switch (ch) 
        {
            case 's':
                ubus_socket = optarg;
                break;
            default:
                break;
        }
    }

    argc -= optind;
    argv += optind;

    uloop_init();
    //M1. Connect to ubus daemon and get the connected ubus context
    ctx = ubus_connect(ubus_socket);

    if (!ctx) 
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }
    //M2. Add the connected fd into epoll fd set
    ubus_add_uloop(ctx);
    // call specific ubus method 
    ubus_cli_call(ctx, argc, argv);

    //When request done, just free the resource, and return
    ubus_free(ctx);
    uloop_done();
    return 0;
}


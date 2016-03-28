/*How to use notification 
//Subscriber(##########demo3##########)
    1. Connect the process to ubus daemon,  
       will get the ubus context, the context will contained the connected fd, 
       registered fd callback and an AVL tree to manage all objects information with this connection
    2. Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set 
    3. Define a subscriber object, which contain a ubus object and  a callback to handle received subscribe notification
    4. Add ubus object onto ubus daemon
    5. Specify callback handler to handle notification
    6. Subscribe interested object(notify object)
*/

static struct ubus_subscriber test_event;
static void subscriber_main(void)
{
    int ret;
    uint32_t id;
    // S4. Add subscriber object onto bus  
    ret = ubus_register_subscriber(ctx, &test_event);
    if (ret)
        fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));
    // S5. Specify callback handler to handle notification
    test_event.remove_cb = test_handle_remove;
    test_event.cb = test_notify;

    // Lookup the notify object
    ret = ubus_lookup_id(ctx, "network.interface", &id);

    // S6.  Subscribe interested object
    ret = ubus_subscribe(ctx, &test_event, id);
    uloop_run();
}

int main(int argc, char **argv)
{
    const char *ubus_socket = NULL;
    int ch;
    while ((ch = getopt(argc, argv, "cs:")) != -1) {
        switch (ch) {
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
    signal(SIGPIPE, SIG_IGN);

    //S1.  Connect the process to ubus daemon
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }
    //S2.  Add connected fd into epoll fd set.
    ubus_add_uloop(ctx);
    // Subscriber main process
    subscriber_main();

    ubus_free(ctx);
    uloop_done();
    return 0;
}

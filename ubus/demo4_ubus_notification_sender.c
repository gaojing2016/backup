/*Notification Sender
N1.  Connect the process to ubus daemon,  will get the ubus context, i
     the context will contained the connected fd, registered fd callback and an AVL tree to manage all objects information with this connection
N2.  Using uloop utilities to add the ubus_context, which is to register the connected fd into epoll set
N3.  Define a notify object
N4.  Add notify object onto bus
N5.  Prepare notify type and arguments when actually an event happens 
N6.  Broadcast the event notification to bus
*/


//N3.  Define a notify object
static struct ubus_object test_object ;
static void event_broadcast(char *event)
{
    //prepare event argument if necessary
    // N6.  Broadcast the event notification to bus
    ubus_notify(ctx,  &test_object, event, NULL, -1);
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
    //N1.  Connect the process to ubus daemon
    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }
    //N2.  Add connected fd into epoll fd set
    ubus_add_uloop(ctx);
    //N4.  Add notify object onto bus
    ubus_add_object(ctx, & test_object);
    //N5.  Prepare notify type and arguments when actually an  event happens 
    ……
        event_ broadcast(event);

    ubus_free(ctx);
    uloop_done();
    return 0;
} 

//The example code can refer to ubus/examples/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int ba2str(const bdaddr_t *ba, char *str)
{
    return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
            ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int main (int argc, char **argv)
{
    struct sockaddr_l2 loc_addr ={0}, rem_addr={0};
    char buf[1024] ={0};//,*addr;
    int s, client, bytes_read, result;
    int opt = sizeof(rem_addr);

    /* allocate socket */
    printf("Creating socket...\n");
    s = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if(s<0)
    {
        perror("create socket error");
        return -1;
    }
    else
    {
        printf("success!\n");
    }
    /* bind socket to port 1 of the first available */
    /* local bluetooth adapter */
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_psm = htobs(0x1001);

    printf("Binding socket...\n");
    result = bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    if(result < 0)
    {
        perror("bind socket error:");
        return -1;
    }
    else
    {
        printf("success!\n");
    }
    /* put socket into listening mode */
#if 0
    result = ba2str(&loc_addr.rc_bdaddr, addr);
    if(result < 0)
    {
        perror("addr convert error");
        return -1;
    }
    printf("local addr is:%s \n", addr);
#endif
    printf("Listen... ");
    result = listen(s, 1);
    if(result < 0)
    {
        printf("error:%d \n", result);
        perror("listen error:");
        return -1;
    }
    else
    {
        printf("requested!\n");
    }

    while(1)
    {
        /* accept one connection */
        printf("Accepting...\n");
        client = accept(s, (struct sockaddr *)&rem_addr, &opt);
        if(client < 0)
        {
            perror("accept error");
            return -1;
        }
        else
        {
            printf("OK!\n");
        }

        ba2str(&rem_addr.l2_bdaddr, buf);
        fprintf(stderr, "accepted connection from %s \n", buf);
        memset(buf, 0, sizeof(buf));
        /* read data from the client */
        //while(1)
        //{
        bytes_read = read(client, buf, sizeof(buf));
        if(bytes_read > 0)
        {
            printf("received[%s]\n", buf);
            if(strcmp(buf, "goodbye") == 0)
            {
                return -1;
            }
            memset(buf, 0, bytes_read);
        }
        //}
    }
    /* close connection */
    close(client);
    close(s);
    return 0 ;
}

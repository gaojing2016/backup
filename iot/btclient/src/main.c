#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int bachk(const char *str)
{
    if (!str)
        return -1;

    if (strlen(str) != 17)
        return -1;

    while (*str) {
        if (!isxdigit(*str++))
            return -1;

        if (!isxdigit(*str++))
            return -1;

        if (*str == 0)
            break;

        if (*str++ != ':')
            return -1;
    }

    return 0;
}

int str2ba(const char *str, bdaddr_t *ba) 
{
    int i;

    if (bachk(str) < 0) { 
        memset(ba, 0, sizeof(*ba));
        return -1;
    }    

    for (i = 5; i >= 0; i--, str += 3)
        ba->b[i] = strtol(str, NULL, 16); 

    return 0;
}



int main( int argc, char **argv)
{
    struct sockaddr_l2 addr = {0};
    int s, status;
    char *dest, *buf; //="00:11:67:32:61:62";
    char *remoteAddr = "00:1A:7D:DA:71:11";

    /* allocate a socket */
    s = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if(s < 0)
    {
        perror("create socket error");
        return -1;
    }
    else
    {
        printf("client create socket succeed\n");
    }

    /* set the connection parameters (who to connect to ) */
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x1001);
    str2ba(remoteAddr, &addr.l2_bdaddr);

    /* connect to server: remote device */
    printf("connectting...\n");
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    /* send a message */
    if(status < 0)
    {
        printf("connect remote device error\n");
    }
    else
    {
        printf("scuess!\n");
        status = write(s, "hello!", 6);

        do{
            scanf("%s", buf);
            status = write(s, buf, strlen(buf));
            if(status < 0) perror("uh oh");
        }while(strcmp(buf, "goodbye") != 0);
    }

    close(s);
    return 0;
}

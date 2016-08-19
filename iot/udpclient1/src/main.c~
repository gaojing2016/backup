#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<stdarg.h>
#include<string.h>

#define SERVER_PORT 7666
#define BUFFER_SIZE 4096
#define FILE_NAME_MAX_SIZE 512

int main(int argc, char *argv[])
{
    /* 服务端地址 */
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(SERVER_PORT);

    /* 创建socket */
    int client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket_fd < 0)
    {
        perror("Create Socket Failed:");
        exit(1);
    }


    /* 发送文件名 */
    char jsBuf[BUFFER_SIZE] = {0};
    char manufacture[10] = "FX";
    char moduleNumber[10] = "022CP";
    char deviceType[10] = "led";
    char specialConfig[10] = "test";
    
    char deviceId[32] = {0};
    printf("argv[2] = %s\n", argv[2]);
    strcpy(deviceId, argv[2]);

    protoDevOnlineBuf(manufacture, moduleNumber, deviceId, deviceType, specialConfig, jsBuf);
    printf("jsBuf = %s\n", jsBuf);
    if(sendto(client_socket_fd, jsBuf, BUFFER_SIZE,0,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
    {
        perror("Send Failed:");
    }
    else
    {
        printf("send succeed\n");
    }

    close(client_socket_fd);
    return 0;
} 

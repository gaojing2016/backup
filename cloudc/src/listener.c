#include "share.h"
#include "parser.h"
#include "cJSON.h"

int cloudc_read_data_from_server(int fd, char *buf);
void process_cloudc_monitor(struct uloop_fd *u, unsigned int events);

struct uloop_fd cloudc_monitor_uloop_fd =
{
    .cb = process_cloudc_monitor,
    .fd = -1
};

void process_cloudc_monitor(struct uloop_fd *u, unsigned int events)
{
    char recv_buf[RECV_MAX_BUF_LEN] = {0};
    int recv_buf_len = 0;
    
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    recv_buf_len = cloudc_read_data_from_server(u->fd, recv_buf);

    if (recv_buf_len > 0)
    { 
        cloudc_parse_receive_info(recv_buf);
    }
    else if (-1 == recv_buf_len)
    {
        cloudc_debug("%s[%d]: recv_buf_len = %d, %s, need to reconnect to server ", __func__, __LINE__, recv_buf_len, strerror(errno));
        close(u->fd);
        socket_init();
        ap_register();
    }
    else
    {
        cloudc_debug("%s[%d]: recv_buf_len = %d, no need to handle ", __func__, __LINE__, recv_buf_len);
    }
    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
}


int cloudc_read_data_from_server(int fd, char *buf)
{
    cloudc_debug("%s[%d]: Enter ", __func__, __LINE__);
    int recv_len = -1;

    recv_len = recv(fd, buf, RECV_MAX_BUF_LEN,0);

    if (recv_len < 0)
    {
        cloudc_debug("%s[%d]: error: %s, recv error, recv_len = %d, fd = %d", __func__, __LINE__, strerror(errno), recv_len, fd);
    }

    cloudc_debug("%s[%d]: Exit ", __func__, __LINE__);
    return recv_len;
}



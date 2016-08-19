#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "cloudc_log.h"

int cloudc_log_level_int = LOG_LEVEL_DEBUG ;
void log_log(LOG_LEVEL log_level, const char *func, unsigned int line_num, const char *p_fmt, ... )
{
    char *log_level_ch_ptr = NULL;

    int len = 0;

    char buf[MAX_LOG_LINE_LENGTH] = {0};

    va_list      ap;

    switch(log_level)
    {
        case LOG_LEVEL_ERR:
            {
                log_level_ch_ptr = "Error ";
                break;
            }

        case LOG_LEVEL_DEBUG:
            {
                log_level_ch_ptr = "Debug ";
                break;
            }

        default:
            {
                log_level_ch_ptr = "Invalid log level ";
            }

            snprintf(buf, MAX_LOG_LINE_LENGTH, "%s", log_level_ch_ptr);

    }

    if ( log_level <= cloudc_log_level_int)
    {
        int log_fd = open("/dev/console",  O_RDWR);

        if ( -1 != log_fd)
        {
            va_start(ap, p_fmt);
            vsnprintf(&(buf[strlen(buf)]), MAX_LOG_LINE_LENGTH, p_fmt, ap);
            write(log_fd, buf, strlen(buf));
            write(log_fd, "\n", strlen("\n"));
            va_end(ap);
            close(log_fd);
        }
    }

    return;
}

#define LOG_EMERG        0      /* system is unusable */
#define LOG_ALERT        1      /* action must be taken immediately */
#define LOG_CRIT         2      /* critical conditions */
#define LOG_ERR          3      /* error conditions */
#define LOG_WARNING      4      /* warning conditions */
#define LOG_NOTICE       5      /* normal but signification condition */
#define LOG_INFO         6      /* informational */
#define LOG_DEBUG        7      /* debug-level messages */



#define cloudc_error(args...)  log_log(LOG_ERR, __FUNCTION__, __LINE__, args)
#define cloudc_debug(args...)  log_log(LOG_DEBUG, __FUNCTION__, __LINE__, args)

#define MAX_LOG_LINE_LENGTH      512

typedef enum
{
    LOG_LEVEL_ERR    = 3, /**< Message at error level. */
    LOG_LEVEL_DEBUG  = 7  /**< Message at debug level. */
} LOG_LEVEL;



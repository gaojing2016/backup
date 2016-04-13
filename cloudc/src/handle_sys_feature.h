#define LEN_OF_CMD_HEAD 20
#define IPK_INSTALLED 0
#define IPK_UNINSTALLED 1


struct FtpFile 
{
    const char *filename;
    FILE *stream;
};

typedef struct ipk_query_info_node 
{
    char ipk_query_name[MAX_IPK_NAME_LEN];
    struct ipk_query_info_node *pNext;
}IPK_QUERY_INFO_NODE;


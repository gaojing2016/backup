#include "share.h"
#include "parser.h"
#include <pthread.h>


void printids(const char *s); 
void *cloudc_process_task_thread(void *arg);
int cloudc_pthread_init(void);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

void printids(const char *s) 
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();

    cloudc_debug("%s[%d]: %s pid %u tid %u (0x%x)\n", __func__, __LINE__, s, (unsigned int) pid, (unsigned int) tid, (unsigned int) tid);
}

void *cloudc_process_task_thread(void *arg)
{
    printids(" new thread: ");
    task_queue_init(& queue_head);

    while(1)
    {
        if(NOT_EMPTY == task_queue_is_empty(&queue_head)) /*  Description: 队列是否为空 */
        {
            cloudc_debug(" hello, I am processing tasks ...");
            /* handle dequeue(task_queue_dequeue)
             * 1.handle data 
             * 2.free node
             */
            task_queue_deque(&queue_head);
        }

        usleep(100*1000);
    }
    return ((void *)0);
}


int cloudc_pthread_init(void)
{
    pthread_t cloudc_tid;
    int ret = -1;

    ret = pthread_create(&cloudc_tid, NULL, cloudc_process_task_thread, NULL);

    cloudc_debug("%s[%d]: ret = %d\n", __func__, __LINE__, ret);

    if(ret != 0)
    {   
        perror("can't create thread: %s\n");
    }  

    printids("main thread: ");
    sleep(1);


    return 0;
}


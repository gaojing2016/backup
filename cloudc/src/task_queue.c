#include "share.h"
#include "parser.h"

struct list_node queue_head;
void task_queue_init(struct list_node *q_head); 
void task_queue_enque(struct list_node *q_head, struct http_value recv_data);
void task_queue_deque(struct list_node *q_head); 
int task_queue_length(struct list_node *q_head);  
int task_queue_is_empty(struct list_node *q_head);
void task_queue_print(struct list_node *q_head);

/* init task queue */
void task_queue_init(struct list_node *q_head)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    memset(q_head, 0, sizeof(struct list_node));

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
}

/* enque from tail */
void task_queue_enque(struct list_node *q_head, struct http_value recv_data)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    struct list_node *q_tmp = q_head;

    struct task_data_node* qn_ptr = (struct task_data_node*)malloc(sizeof(struct task_data_node));

    if(NULL == qn_ptr)
    {
        cloudc_error("%s[%d]: malloc failed", __func__, __LINE__);
        return;
    }
    memset(qn_ptr, 0, sizeof(struct task_data_node));

    memcpy(&(qn_ptr->data), &recv_data, sizeof(qn_ptr->data));


    while(NULL != q_tmp->next)
    {
        q_tmp = q_tmp->next;
    }

    q_tmp->next = &(qn_ptr->task_list_node);

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

/* dequeue from head : firstly handle data then free the node */
void task_queue_deque(struct list_node *q_head)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    struct task_data_node *qn_tmp_ptr;
    struct task_data_node *queue_node;

    if (NULL == q_head->next)
    {
        cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
        return;
    }

    else
    {
        /* handle data part of the node */
        cloudc_rpc_method_handle(((struct task_data_node*)(q_head->next))->data); 

        /* free node*/
        q_head->next = q_head->next->next;
        free((struct task_data_node*)(q_head->next));
    }

    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

/* print info in task queue, just for debug */
void task_queue_print(struct list_node *q_head)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    struct list_node *q_tmp = q_head;
    while(NULL != q_tmp->next)
    {
        cloudc_debug("%s[%d]: register_status = %d\n", __func__, __LINE__, ((struct task_data_node*)(q_tmp->next))->data.register_status);
        q_tmp = q_tmp->next;
    }
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return;
}

/* to know how many tasks in the queue */
int task_queue_length(struct list_node *q_head)
{
    cloudc_debug("%s[%d]: Enter", __func__, __LINE__);

    int count = 0;
    struct list_node *q_tmp = q_head;

    while(NULL != q_tmp->next)
    {
        count ++;
        q_tmp = q_tmp->next;
    }

    cloudc_debug("%s[%d]: task_queue_length = %d\n", __func__, __LINE__, count);
    cloudc_debug("%s[%d]: Exit", __func__, __LINE__);
    return count;
}

/* decide if the queue is empty or not */
int task_queue_is_empty(struct list_node *q_head)
{
    if(NULL == q_head->next)
    {
        return EMPTY;
    }
    else
    {
        return NOT_EMPTY;
    }
}


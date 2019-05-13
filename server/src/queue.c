#include "../include/function.h"
int queueini(thrqueue *q)
{
    memset(q,0,sizeof(thrqueue));
    pthread_mutex_init(&q->mutex,NULL);
    q->size = 0;
    q->front = NULL;
    q->rear = NULL;
    return 0;
}
int dequeue(thrqueue *q, node *out)
{
    if(q->size > 1)
    {
        out->fd = q->front->fd;
        q->front = q->front->next;
        q->size --;
    }
    else if(q->size == 1)
    {
        out->fd = q->front->fd;
        q->front = NULL;
        q->rear = NULL;
        q->size = 0;
    }
    else
    {
        printf("queue error\n");
        exit(-1);
    }
}
int enqueue(thrqueue *q,node *in)
{
    if(q->size == 0)
    {
        q->front = in;
        q->rear = in;
        q->size = 1;
    }
    else
    {
        q->rear->next = in;
        q->rear = in;
        q->size ++;
    }
}


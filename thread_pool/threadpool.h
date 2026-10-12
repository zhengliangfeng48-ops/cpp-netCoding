#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<assert.h>


struct threadpool_task_t{
    void *(*function)(void *);
    void *arg;
};

struct threadpool_t{
    pthread_mutex_t lock;               //锁住本结构体
    pthread_mutex_t thread_counter;     //记录忙状态线程个数的锁

    pthread_cond_t queue_not_full;      //任务队列满时等待此条件变量
    pthread_cond_t queue_not_empty;     //空时等待

    pthread_t *threads;                 //数组：线程池中每个线程的tid
    pthread_t adjust_tid;               //管理线程tid
    threadpool_task_t *task_queue;      //任务队列

    int min_thr_num;                    //线程池最小线程数
    int max_thr_num;                    //最大
    int live_thr_num;                   //存活线程数
    int busy_thr_num;                   //忙状态线程数
    int wait_exit_thr_num;              //要销毁的线程数

    int queue_front;                    //task_queue队头下标
    int queue_rear;                     //队尾
    int queue_size;                     //队中实际任务数
    int queue_max_size;                 //队列可容纳任务数上限

    int shutdown;                       //标志位，线程池使用状态，true / false
};

threadpool_t* threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);

void *threadpool_thread(void* threadpool);

void *adjust_thread(void* threadpool);

int threadpoll_add(threadpool_t *pool,void* (*function)(void *arg),void *arg);

void *process(void* arg);

int threadpool_destory(threadpool_t *pool);

int threadpool_free(threadpool_t *pool);
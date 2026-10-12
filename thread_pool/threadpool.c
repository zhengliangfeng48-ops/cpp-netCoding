#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<assert.h>
#include "threadpool.h"

#define DEFAULT_TIME 10
#define MIN_WAIT_TASK_NUM 10
#define DEFAULT_THREAD_VARY 10
#define true 1
#define false 0

typedef struct{
    void *(*function)(void *);
    void *arg;
} threadpool_task_t;

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

threadpool_t* threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size){
    int i;
    threadpool_t *poll=NULL;
    
    do{
        if((pool=(threadpool_t*)malloc(sizeof(threadpool_t)))==NULL){
            printf("malloc threadpool fail\n");
            break;
        }
        pool->min_thr_num=min_thr_num;
        pool->max_thr_num=max_thr_num;
        pool->busy_thr_num=0;
        pool->live_thr_num=min_thr_num;
        pool->wait_exit_thr_num=0;
        pool->queue_size=0;
        pool->queue_max_size=queue_max_size;
        pool->queue_front=0;
        pool->queue_rear=0;
        pool->shutdown=false;

        //工作线程数组初始化
        pool->threads=(pthread_t*)malloc(sizeof(pthread_t)*max_thr_num);
        if(pool->threads==NULL){
            printf("malloc threads fail");
            break;
        }
        memset(pool->threads,0,sizeof(pthread_t)*max_thr_num);

        //任务队列初始化
        pool->task_queue=(threadpool_task_t*)malloc(sizeof(threadpool_task_t)*queue_max_size);
        if(pool->task_queue==NULL){
            printf("malloc task_queue fail");
            break;
        }

        if(pthread_mutex_init(&(pool->lock),NULL)!=0
            ||pthread_mutex_init(&(pool->thread_counter),NULL)!=0
            ||pthread_cond_init(&(pool->queue_not_empty),NULL)!=0
            ||pthread_cond_init(&(pool->queue_not_full),NULL)!=0){

            printf("init the lock or cond fail\n");
            break;
        }

        //启动初始线程
        for(i=0;i<min_thr_num;++i){
            pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
            printf("start thread 0x%x...\n",(unsigned int)pool->threads[i]);
        }
        pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void*)pool); //管理线程

        return pool;

    }while(false);          //顶替goto，上方可用break跳转至此

    threadpool_free(pool);  //失败释放pool
    return NULL;
}

int main(){
    threadpool_t *thp=threadpool_create(3,100,100); //最小3线程，最大100进程，队列最大100任务
    printf("pool inited\n");

    int num[20],i;
    for(i=0;i<20;++i){
        num[i]=i;
        printf("add task %d\n",i);

        threadpool_add(thp,process,(void*)&num[i]); //添加任务
    }
    sleep(10);  //等子线程完成任务
}
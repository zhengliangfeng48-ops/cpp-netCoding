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

typedef struct threadpool_t{
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
} threadpool_t;

threadpool_t* threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size){
    int i;
    threadpool_t *pool=NULL;
    
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

//工作线程
void *threadpool_thread(void* threadpool){
    threadpool_t *pool=(threadpool_t*)threadpool;
    threadpool_task_t task;

    while(true){
        //等待任务
        pthread_mutex_lock(&(pool->lock));
        
        //没任务阻塞在条件变量，有任务跳过此while
        while((pool->queue_size==0)&&(!pool->shutdown)){
            printf("thread 0x%x is waiting\n",(unsigned int)pthread_self());
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));

            //清除指定数目的空闲线程
            if(pool->wait_exit_thr_num>0){
                pool->wait_exit_thr_num--;

                if(pool->live_thr_num>pool->min_thr_num){
                    //存活线程数大于最小值时可结束当前线程
                    printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));

                    pthread_exit(NULL);
                }
            }
        }

        if(pool->shutdown){
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        //从任务队列获取任务
        task.function=pool->task_queue[pool->queue_front].function;
        task.arg=pool->task_queue[pool->queue_front].arg;
        pool->queue_front=(pool->queue_front+1)%pool->queue_max_size;
        pool->queue_size--;

        //通知可以有新的任务入队
        pthread_cond_broadcast(&(pool->queue_not_full));

        pthread_mutex_unlock(&(pool->lock));

        //执行任务
        printf("thread 0x%x start working\n",(unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num++;
        pthread_mutex_unlock(&(pool->thread_counter));

        (*(task.function))(task.arg);
        //task.function(task.arg);

        //任务结束处理
        printf("thread 0x%x end working\n",(unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->thread_counter));
    }
}

//管理线程
void *adjust_thread(void* threadpool){
    threadpool_t *pool=(threadpool_t*)threadpool;
    int i;

    while(!pool->shutdown){
        sleep(DEFAULT_TIME);        //定时对线程池管理

        pthread_mutex_lock(&pool->lock);
        int queue_size=pool->queue_size;
        int live_thr_num=pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num=pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        //任务数 大于 最小线程个数 ，且 存活的线程数 少于 最大线程个数 时，扩容线程池
        if(queue_size>=MIN_WAIT_TASK_NUM && live_thr_num<pool->max_thr_num){
            pthread_mutex_lock(&(pool->lock));
            int add=0;

            for(i=0;i<pool->max_thr_num && add<DEFAULT_THREAD_VARY
                    && pool->live_thr_num<pool->max_thr_num;++i){
                if(pool->threads[i]==0||!is_thread_alive(pool->threads[i])){
                    pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
                    ++add;
                    pool->live_thr_num++;
                }
            }
            pthread_mutex_unlock(&(pool->lock));
        }

        //忙线程数 不到 存活线程数 的一半，且 存活线程数 大于 最小线程数 时，销毁空闲线程
        if((busy_thr_num*2)<live_thr_num && live_thr_num >pool->min_thr_num){
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num=DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));

            for(i=0;i<DEFAULT_THREAD_VARY;++i){
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
}

//模拟产生任务
int threadpoll_add(threadpool_t *pool,void* (*function)(void *arg),void *arg){
    pthread_mutex_lock(&(pool->lock));

    //队满阻塞
    while((pool->queue_size==pool->queue_max_size)&&(!pool->shutdown)){
        pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
    }

    if(pool->shutdown){
        pthread_cond_broadcast(&(pool->queue_not_empty));
        pthread_mutex_unlock(&(pool->lock));
    }

    //清空工作线程回调函数参数
    if(pool->task_queue[pool->queue_rear].arg!=NULL){
        pool->task_queue[pool->queue_rear].arg=NULL;
    }

    //添加任务到队里
    pool->task_queue[pool->queue_rear].function=function;
    pool->task_queue[pool->queue_rear].arg=arg;
    pool->queue_rear=(pool->queue_rear+1)%pool->queue_max_size;
    pool->queue_size++;

    //唤醒等待处理任务的线程
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));
}

//任务函数
void *process(void* arg){
    printf("thread 0x%x working on task %d\n",(unsigned int)pthread_self(),(int)arg);
    sleep(1);
    printf("task %d is end\n",(int)arg);
}

int threadpool_destory(threadpool_t *pool){
    int i;
    if(pool==NULL)
        return -1;

    pool->shutdown=true;

    //销毁管理线程
    pthread_join(pool->adjust_tid,NULL);

    for(i=0;i<pool->live_thr_num;++i){
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for(i=0;i<pool->live_thr_num;++i){
        pthread_join(pool->threads[i],NULL);
    }
    threadpool_free(pool);

    return 0;
}

int threadpool_free(threadpool_t *pool){
    if(pool==NULL)
        return -1;

    if(pool->task_queue)
        free(pool->task_queue);

    if(pool->threads){
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool=NULL;

    return 0;
}

int main(){
    threadpool_t *thp=threadpool_create(3,100,100); //最小3线程，最大100进程，队列最大100任务
    printf("pool inited\n");

    int num[20];
    for(int i=0;i<20;++i){
        num[i]=i;
        printf("add task %d\n",i);

        threadpool_add(thp,process,(void*)&num[i]); //添加任务
    }
    sleep(10);  //等子线程完成任务
    threadpool_destory(thp);
}

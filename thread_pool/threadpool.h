#ifndef THREAD_POOL_HEAD
#define THREAD_POOL_HEAD

#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<assert.h>

#define DEFAULT_TIME 10
#define MIN_WAIT_TASK_NUM 10
#define DEFAULT_THREAD_VARY 10
#define true 1
#define false 0

typedef struct threadpool_t threadpool_t;

typedef struct threadpool_task_t threadpool_task_t;

threadpool_t* threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);

void *threadpool_thread(void* threadpool);

void *adjust_thread(void* threadpool);

int threadpoll_add(threadpool_t *pool,void* (*function)(void *arg),void *arg);

void *process(void* arg);

int threadpool_destory(threadpool_t *pool);

int threadpool_free(threadpool_t *pool);

bool is_thread_alive(pthread_t tid);

#endif 
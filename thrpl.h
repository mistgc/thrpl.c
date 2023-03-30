#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <pthread.h>

#define MAX_NUMBER_OF_THREADS 20
#define MIN_NUMBER_OF_THREADS 4
#define MAX_NUMBER_OF_TASKS 256
// #define MIN_NUMBER_OF_WAIT_TASKS 4
#define DEFAULT_NUMBER_OF_THREAD 4

typedef void *(*function)(void *);

typedef struct {
  function func;
  void *argv;
} Task;

typedef struct {
  int head;
  int rear;
  int count;
  int size;
  Task *tasks;
} TaskQueue;

TaskQueue *TaskQueue_new();
int TaskQueue_enqueue(TaskQueue *self, Task task);
int TaskQueue_dequeue(TaskQueue *self, Task *out_task);

typedef struct {
  TaskQueue *task_queue;
  pthread_mutex_t mutex;
  pthread_mutex_t thr_counter;
  pthread_cond_t queue_not_full;
  pthread_cond_t queue_not_empty;

  pthread_t admin_tid;
  pthread_t *threads;
  size_t busy_thr_num;
  size_t live_thr_num;
  size_t min_thr_num;
  size_t max_thr_num;
  size_t wait_exit_thr_num;

  int shutdown;
} ThreadPool;

ThreadPool *ThreadPool_new();
void *ThreadPool_thread(void *thp);
void *ThreadPool_admin_thread(void *thp);
int ThreadPool_add_task(ThreadPool *self, Task task);
int ThreadPool_destroy(ThreadPool *self);
int ThreadPool_free(ThreadPool *self);

#endif // _THREAD_POOL_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "thrpl.h"

#define DEFAULT_TIME 2

#define GET_TASK_COUNT(pool) (pool->task_queue->count)
#define GET_TASK_SIZE(pool) (pool->task_queue->size)
#define GET_TASK_HEAD(pool) (pool->task_queue->head)
#define GET_TASK_REAR(pool) (pool->task_queue->rear)
#define DE_QUEUE(pool, out_task) (TaskQueue_dequeue(pool->task_queue, out_task))
#define EN_QUEUE(pool, task) (TaskQueue_enqueue(pool->task_queue, task))
// #define IS_THREAD_ALIVE(tid) ((pthread_kill(tid, 0) == ESRCH) ? 0 : 1)

TaskQueue *TaskQueue_new() {
  TaskQueue *ret = (TaskQueue *)malloc(sizeof(TaskQueue));
  *ret = (TaskQueue){
      .head = 0,
      .rear = 0,
      .count = 0,
      .size = MAX_NUMBER_OF_TASKS,
      .tasks = (Task *)malloc(sizeof(Task) * MAX_NUMBER_OF_TASKS),
  };

  return ret;
}

int TaskQueue_enqueue(TaskQueue *self, Task task) {
  if (self == NULL) {
    fprintf(stderr, "[Error] The pointer to task queue is invalid.\n");
    return -1;
  }
  if ((self->rear + 1) % self->size == self->head) {
    fprintf(stderr, "[Error] Task queue is full, can not add a new task.\n");
    return -1;
  }
  self->rear = (self->rear + 1) % self->size;
  self->tasks[self->rear] = task;
  self->count++;
  return 0;
}

int TaskQueue_dequeue(TaskQueue *self, Task *out_task) {
  if (self == NULL) {
    fprintf(stderr, "[Error] The pointer to task queue is invalid.\n");
    return -1;
  }
  if (self->head == self->rear) {
    fprintf(stderr, "[Error] Task queue is empty, can not remove a task.\n");
    return -1;
  }
  self->head = (self->head + 1) % self->size;
  *out_task = self->tasks[self->head];
  self->count--;
  return 0;
}

void TaskQueue_free(TaskQueue *self) {
  if (self != NULL) {
    free(self->tasks);
    free(self);
  }
}

ThreadPool *ThreadPool_new() {
  ThreadPool *pool = NULL;
  if ((pool = (ThreadPool *)malloc(sizeof(ThreadPool))) == NULL) {
    fprintf(stderr, "[Error] malloc thread pool failed!");
    perror("malloc");
    return NULL;
  }

  *pool = (ThreadPool){
      .task_queue = TaskQueue_new(),
      .threads = (pthread_t *)malloc(sizeof(pthread_t) * MAX_NUMBER_OF_THREADS),
      .min_thr_num = MIN_NUMBER_OF_THREADS,
      .max_thr_num = MAX_NUMBER_OF_THREADS,
      .busy_thr_num = 0,
      .live_thr_num = MIN_NUMBER_OF_THREADS,
      .wait_exit_thr_num = 0,
      .shutdown = 0,
  };

  if (pthread_mutex_init(&pool->mutex, NULL) != 0 ||
      pthread_mutex_init(&pool->thr_counter, NULL) != 0 ||
      pthread_cond_init(&pool->queue_not_full, NULL) != 0 ||
      pthread_cond_init(&pool->queue_not_empty, NULL) != 0) {
    perror("pthread_mutex_init");
  }

  for (int i = 0; i < pool->min_thr_num; i++) {
    pthread_create(&pool->threads[i], NULL, ThreadPool_thread, (void *)pool);
    printf("[Thread Pool] start thread 0x%x...\n", (uint)pool->threads[i]);
  }

  pthread_create(&pool->admin_tid, NULL, ThreadPool_admin_thread, (void *)pool);

  return pool;
}

void *ThreadPool_thread(void *thp) {
  ThreadPool *pool = thp;
  pthread_t tid = pthread_self();
  Task task;

  while (1) {
    pthread_mutex_lock(&pool->mutex);

    while (GET_TASK_COUNT(pool) == 0 && (!pool->shutdown)) {
      printf("[Thread] thread 0x%x is waiting\n", (uint)tid);
      pthread_cond_wait(&pool->queue_not_empty, &pool->mutex);

      // Thread Suicide
      if (pool->wait_exit_thr_num > 0) {
        pool->wait_exit_thr_num--;
        if (pool->live_thr_num > pool->min_thr_num) {
          printf("[Thread] thread 0x%x is exiting\n", (uint)tid);
          pool->live_thr_num--;
          pthread_mutex_unlock(&pool->mutex);
          for (int i = 0; i < pool->max_thr_num; i++) {
              if (pool->threads[i] == tid) {
                  pthread_detach(tid);
                  pool->threads[i] = 0;
                  break;
              }
          }
          pthread_exit(NULL);
        }
      }
    }

    // Thread Pool Has Shut Down.
    if (pool->shutdown) {
      pthread_mutex_unlock(&pool->mutex);
      printf("[Thread] thread 0x%x is exiting\n", (uint)pthread_self());
      pthread_exit(NULL);
    }

    // Get Task
    DE_QUEUE(pool, &task);
    pthread_cond_broadcast(&pool->queue_not_full);
    // Unlock Mutex
    pthread_mutex_unlock(&pool->mutex);
    printf("[Thread] thread 0x%x start working\n", (uint)pthread_self());
    pthread_mutex_lock(&pool->thr_counter);
    pool->busy_thr_num++;
    pthread_mutex_unlock(&pool->thr_counter);

    // Execute Task
    task.func(task.argv);

    // After Finished Task
    printf("[Thread] thread 0x%x end working\n", (uint)pthread_self());
    pthread_mutex_lock(&pool->thr_counter);
    pool->busy_thr_num--;
    pthread_mutex_unlock(&pool->thr_counter);
  }

  pthread_exit(NULL);
}

int ThreadPool_add_task(ThreadPool *self, Task task) {
  pthread_mutex_lock(&self->mutex);

  while (GET_TASK_COUNT(self) == GET_TASK_SIZE(self) && !self->shutdown) {
    pthread_cond_wait(&self->queue_not_full, &self->mutex);
  }

  // Thread Pool Has Shut Down.
  if (self->shutdown) {
    pthread_mutex_unlock(&self->mutex);
    return -1;
  }

  // Add Task
  EN_QUEUE(self, task);

  // Wake A Thread
  pthread_cond_signal(&self->queue_not_empty);
  pthread_mutex_unlock(&self->mutex);

  return 0;
}

void *ThreadPool_admin_thread(void *thp) {
  ThreadPool *pool = thp;

  while (!pool->shutdown) {
    sleep(DEFAULT_TIME);

    pthread_mutex_lock(&pool->mutex);
    int task_count = GET_TASK_COUNT(pool);
    int live_thr_num = pool->live_thr_num;
    pthread_mutex_unlock(&pool->mutex);

    pthread_mutex_lock(&pool->thr_counter);
    int busy_thr_num = pool->busy_thr_num;
    pthread_mutex_unlock(&pool->thr_counter);

    printf("[Admin] --[busy-%d]--[live-%d]--\n", busy_thr_num, live_thr_num);
    if (task_count > live_thr_num &&
        live_thr_num < pool->max_thr_num) {
      printf("[Admin] ----------add----------\n");

      // Create A Default Number of Thread
      pthread_mutex_lock(&pool->mutex);
      for (int i = 0, j = 0;
           i < pool->max_thr_num && j < DEFAULT_NUMBER_OF_THREAD &&
           pool->live_thr_num < pool->max_thr_num;
           i++) {
        if (pool->threads[i] == 0) {
          pthread_create(&pool->threads[i], NULL, ThreadPool_thread,
                         (void *)pool);
          j++;
          pool->live_thr_num++;
          printf("[Admin] -------new thread------\n");
        }
      }
      pthread_mutex_unlock(&pool->mutex);
    }
    // Clear Idle Threads
    if (busy_thr_num * 2 < live_thr_num && live_thr_num > pool->min_thr_num) {
      pthread_mutex_lock(&pool->mutex);
      pool->wait_exit_thr_num = DEFAULT_NUMBER_OF_THREAD;
      pthread_mutex_unlock(&pool->mutex);

      for (int i = 0; i < DEFAULT_NUMBER_OF_THREAD; i++) {
        pthread_cond_signal(&pool->queue_not_empty);
        printf("[Admin] ----------clear--------\n");
      }
    }
  }

  return NULL;
}

int ThreadPool_destroy(ThreadPool *self) {
  if (self == NULL) {
    return -1;
  }
  self->shutdown = 1;
  pthread_join(self->admin_tid, NULL);

  for (int i = 0; i < self->live_thr_num; i++) {
    pthread_cond_broadcast(&self->queue_not_empty);
  }

  for (int i = 0; i < self->max_thr_num; i++) {
      if (self->threads[i] != 0) {
        pthread_join(self->threads[i], NULL);
      }
  }

  ThreadPool_free(self);

  return 0;
}

int ThreadPool_free(ThreadPool *self) {
  if (self == NULL) {
    return -1;
  }
  if (self->task_queue) {
    TaskQueue_free(self->task_queue);
  }
  if (self->threads) {
    free(self->threads);
    pthread_mutex_lock(&self->mutex);
    pthread_mutex_destroy(&self->mutex);
    pthread_mutex_lock(&self->thr_counter);
    pthread_mutex_destroy(&self->thr_counter);
    pthread_cond_destroy(&self->queue_not_empty);
    pthread_cond_destroy(&self->queue_not_full);
  }
  free(self);

  return 0;
}

#undef DEFAULT_TIME
#undef GET_TASK_COUNT
#undef GET_TASK_SIZE
#undef GET_TASK_HEAD
#undef GET_TASK_REAR
#undef DE_QUEUE
#undef EN_QUEUE
// #undef IS_THREAD_ALIVE

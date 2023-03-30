#include "thrpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *taskFunc(void *arg) {
  int num = *(int *)arg;
  printf("[Test] thread 0x%x is working, number = %d\n", (uint)pthread_self(),
         num);
  sleep(1);
  free(arg);
  return NULL;
}

int main() {
  ThreadPool *pool = ThreadPool_new();
  for (int i = 0; i < 100; ++i) {
    int *num = (int *)malloc(sizeof(int));
    *num = i + 100;
    Task task = {
        .func = taskFunc,
        .argv = num,
    };
    ThreadPool_add_task(pool, task);
  }

  sleep(20);

  for (int i = 0; i < 100; ++i) {
    int *num = (int *)malloc(sizeof(int));
    *num = i + 1000;
    Task task = {
        .func = taskFunc,
        .argv = num,
    };
    ThreadPool_add_task(pool, task);
  }

  sleep(30);

  ThreadPool_destroy(pool);

  return 0;
}

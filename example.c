// #define THRPL_IMPLEMENTATION

// #include "header-only/thrpl.h"
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
    int ret = 0;
    int *num = (int *)malloc(sizeof(int));
    *num = i + 100;
    Task task = {
        .func = taskFunc,
        .argv = num,
    };
    if (-1 == (ret = ThreadPool_add_task(pool, task))) {
      free(num);
    }
  }

  sleep(20);

  for (int i = 0; i < 100; ++i) {
    int ret = 0;
    int *num = (int *)malloc(sizeof(int));
    *num = i + 1000;
    Task task = {
        .func = taskFunc,
        .argv = num,
    };
    if (-1 == (ret = ThreadPool_add_task(pool, task))) {
      free(num);
    }
  }

  ThreadPool_gracefully_destroy(pool);

  return 0;
}

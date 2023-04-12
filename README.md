<div align="center">
    <h1>thrpl.c</h1>
    <p>An implementation of Thread Pool in C.</p>
</div>

## Run an example

The thread pool uses POSIX threads, so compile using the `-lpthread` flag.

```bash
gcc example.c thrpl.c -lpthread -o example && ./example
```

Debug mode:

```bash
gcc example.c thrpl.c -lpthread -DTHRPL_DEBUG -o example && ./example
```

## Usage

```bash
git clone https://github.com/zaiic/thrpl.c
cd thrpl.c
```

Put `thrpl.c` and `thrpl.h` in your project or generate a **header-only** file and put it in your project.

Generate **header-only** file:

```bash
./generate_header_only.sh
```

Notice: The *header-only* file is [stb style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).
Don't forget to put `#define THRPL_IMLEMENTATION` in your file before including *header-only* file.

```c
#define THRPL_IMLEMENTATION

#include "thrpl.h" // header-only file
```

## Api

| Function | Description |
|---|---|
| ThreadPool *ThreadPool_new() | Create a new thread pool and return a pointer to it. |
| int ThreadPool_add_task(ThreadPool *self, Task task) | Add a task to the thread pool. |
| int ThreadPool_gracefully_destroy(ThreadPool *self) | Destroy the thread pool when the task queue is empty. |
| int ThreadPool_destroy(ThreadPool *self) | Destroy the thread pool. |

```c
typedef void *(*function)(void *);

typedef struct {
  function func;
  void *argv;
} Task;
```

## License

[The MIT License](./LICENSE)

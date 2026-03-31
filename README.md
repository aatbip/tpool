# Tpool

Tpool is a high-performance thread pool library written in C. I am writing Tpool to better understand concurrency concepts 
and do some experiments with multithreaded programming. I have plans to further improve Tpool which I will discuss in the "Goals" section below.

### Overview

Tpool uses bounded (circular) buffer to store the struct type `job`. 

```C
typedef struct _job {
  job_func *f;
  void *arg;
} job;
```
The struct `job` has pointer to the job function `job_func` and the argument pointer to the function `arg`. This struct is 
stored in the circular buffer which has operations based on a queue ADT. Tpool uses first-in-first-out (FIFO) scheduling
policy, in other words, the first job enqueued in the circular buffer is dequeued at first for processing.

Tpool uses single producer multi consumer approach. A single thread will be responsible for adding jobs to the queue while
multiple worker threads are created to process the job.

Tpool uses the Linux pthread API to create threads, mutex and condition variables to achieve lock based synchronization.

### Tpool API

Below are the list of APIs Tpool provides-

| Function | Detail |
|:------|:---|
|`tpool *tpool_create(int thread_count, int buffer_size)` | Creates threads and returns the pointer of type `tpool` if succeeds, returns NULL if failed. Accepts number of threads `thread_count` and size of the circular buffer `buffer_size` as parameters. |
|`int tpool_add(tpool *tp, void (*func)(void *), void *arg)` |Add jobs to the buffer using `tpool_add` API. It accepts `tpool*`, pointer to the job function and argument of the job function.|
|`void tpool_wait(tpool *tp)` | Use this to wait for worker threads to return. Otherwise, main thread can return without waiting for the worker threads to return resulting in the termination of the program. |
|`int tpool_shutdown(tpool *tp)`|This API makes Tpool to return without executing jobs further. This works on graceful shutdown policy by waiting until working threads return.|
|`int tpool_destroy(tpool *tp)`|Wait for worker threads to return, free the allocated memory, and destroy condition variables and mutex.|
|`int tpool_count_working_threads(tpool *tp)`|Get the count of working threads i.e. threads that are currently processing on jobs.|

### Benchmark

I experimented on Tpool by using it to concurrently run the recursive function of the quicksort algorithm. Program for this experiment can be found
in `/tests/ex1.c` file.

All experiments were executed on the same environment under identical conditions. The system detail is as follows:
| | |
|:---|:---|
|**OS:**|Ubuntu 22.04.5 LTS|
|**Kernel:**|Linux kernel 6.8.0-85-generic|
|**CPU:**|Intel Core i7-10750H CPU (6 core, 12 threads)|
|**Memory:**|16 GB DDR4|
|**Compiler:**|GCC 11.4.0 with -O2 optimization|

**Test I:** Tpool thread pool was created with `thread_count=6` and `buffer_size=50`. Memory was allocated of size `sizeof(int) * s`, where `s=999999999` 
which was then initialized by integer from `0 to s-1`. This array of integer was provided as input to the quicksort sorting algorithm.
Recursive calls of the quicksort algorithm was then pushed as jobs to the buffer using `tpool_add` function which was processed by tpool workers concurrently.
Below is the output of the program `/tests/ex1.c`:




# Tpool

Tpool is a high-performance thread pool library written in C. I am writing Tpool to better understand concurrency concepts 
and do some experiments with multithreaded programming. I have plans to further improve Tpool which I will discuss in the section below.

### Overview

Tpool uses bounded (circular) buffer to store the struct type `job`. 

```C
typedef struct _job {
  job_func *f;
  void *arg;
} job;
```
The struct `job` has pointer to the job function `job_func` and the pointer to the argument `arg` of the job function. This struct is 
stored in the circular buffer which has operations based on a queue ADT. Tpool uses first-in-first-out (FIFO) scheduling
policy, in other words, the first job enqueued in the circular buffer is dequeued at first for processing.

Tpool uses single producer multi consumer approach. A single thread will be responsible for adding jobs to the queue while
multiple worker threads are created to process the job.

Tpool uses the Linux pthread API to create threads, mutex and condition variables to achieve lock based synchronization.

## Experiments and Benchmarks

I experimented Tpool under various workloads. One of such workloads were to make the recursive calls of the quicksort algorithm concurrent. 
I was able to notice the impact of design decision taken by the threadpool library tpool when using it to concurrently run the recursive function 
of the quicksort algorithm. Program for this experiment can be found in `/tests/ex1.c` file. I will also discuss a deadlock bug I came across, the reason
behind such bug, and how did I overcome it in this experiment.

All experiments were executed on the same environment under identical conditions. The system detail is as follows:

|  |  |
|:----|:------|
| **OS:** | Ubuntu 22.04.5 LTS |
| **Kernel:** | Linux kernel 6.8.0-85-generic |
| **CPU:** | Intel Core i7-10750H CPU (6 core, 12 threads) |
| **Memory:** | 16 GB DDR4 |
| **Compiler:** | GCC 11.4.0 with -O2 optimization |

### Test I
Threads were created with `thread_count=6` and `buffer_size=50` using `tpool_create`. Memory buffer of `size(s) = 999999999 * sizeof(int)` was allocated
which was then initialized with integer from `0 to s-1`. This array of integer of size ~4G was provided as input to the quicksort sorting algorithm.
Recursive calls of the quicksort algorithm was then pushed as jobs to the buffer using `tpool_add` function which was processed by tpool workers concurrently.
Below is the output of the program `/tests/ex1.c` using tpool for concurrency:
```sh
>> make run EX=ex1.c               
rm -rf ./tests/out && rm -rf ./tests/tpool.o
gcc -Wall -c tpool.c -o tests/tpool.o
gcc -Wall -I. -pthread tests/ex1.c tests/tpool.o -o tests/out

-------Running ./tests/ex1.c-------

./tests/out
before sort nums[first]: 999999999
before sort nums[last]: 1
Operation took 14504.181 ms
after sort nums[first] 1
after sort nums[last] 999999999
```
Result: It took 14504.181 ms (approx 14 seconds) to sort the array of size `s` (~4GB) in a concurrent manner.

### Test II
The same array initialized above was sorted using the quicksort algorithm without using tpool. Below is the output of the program `/tests/ex1.c`
without using tpool: 

```sh
>> make run EX=ex1.c
rm -rf ./tests/out && rm -rf ./tests/tpool.o
gcc -Wall -c tpool.c -o tests/tpool.o
gcc -Wall -I. -pthread tests/ex1.c tests/tpool.o -o tests/out

-------Running ./tests/ex1.c-------

./tests/out
before sort nums[first]: 999999999
before sort nums[last]: 1
Operation took 43424.730 ms
after sort nums[first] 1
after sort nums[last] 999999999
```
Result: It took 43424.730 ms (approx 43 seconds) to sort the array of size `s` (~4GB) using single thread.

### Observation
It was observed that by using a threadpool library we were able to run recursive functions of the quicksort algorithm concurrently achieving
67% increase in performance while sorting array of integers of size ~4GB. 

### Deadlock bug 
While working on this experiment, I caught a critical deadlock bug that was caused due to concurrently executing the recursive functions.

#### **Cause:**
Tpool uses bounded buffer of fix size and blocking producer/consumer. 

Blocking producer/consumer means that producer can wait and signal consumer if buffer is full. Similarly, consumer can wait and signal producer if buffer is empty
and there are no jobs to execute.

In a recursive quicksort algorithm, we are making the recursive calls concurrent using Tpool.
```c
quicksort(args){
    tpool_add(quicksort, args);
    tpool_add(quicksort, args);
}
```
Pseudocode above shows that `quicksort` function recursively calls `tpool_add` that enqueues the pointer to the function `quicksort` and arguments `args` into the
buffer. Now when the worker threads of tpool run, they again call the same `quicksort` recursive function from the buffer. Which will in turn run `tpool_add`. This
is a classic example of ***worker thread become producers*** in a recursive function making to work concurrently using tpool's design.

This behaviour makes the buffer full making the producer thread to wait. Since consumer (worker) threads also recursively is running `tpool_add` acting like
producer, they also wait. Both threads wait causing ***deadlock***.

#### **Fix:**
As a workaround to using tpool in recursive functions we introduce the per job `depth` counter that defines the max concurrent recursive level. In every 
recursive calls, we decrement the value of `depth` before adding the recursive job to the queue. If `depth` becomes `0` then we will no longer add the recursive job to the queue using `tpool_add` but instead call the recursive function directly. Pseudocode below shows this approach:
```C
quicksort(args){
    if(args->depth <= 0){
        quicksort(args);
        quicksort(args);
    }else {
        args1->depth = args->depth-1; 
        tpool_add(quicksort, args1);
        args2->depth = args->depth-1;
        tpool_add(quicksort, args2);
    }
}
```

We can also use a dynamically sized linkedlist or work stealing deque to design the buffer which should eliminate such issues in recursive functions.

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

### Further Development

I will be working on Tpool further in the upcoming days. Some of the things I want to do are: 
- Aggressive shutdown
- Implement work stealing deque by replacing the bound buffer
- Implement lock free and atomics based data structure (work stealing deque)
- NUMA aware stuffs
- and other ideas still in my mind but aren't solid yet... will add if it interests me.

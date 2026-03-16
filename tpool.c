#include "tpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 50 // Should be able to declare during tpool_create?

#define ERR(s, ...) (fprintf(stderr, "err: " s, ##__VA_ARGS__))

typedef void *job_func(void *);

typedef struct _job {
  job_func *f;
  void *arg;
} job;

typedef struct _tpool {
  int thread_count;
  pthread_t *threads;
  pthread_mutex_t mutex;
  pthread_cond_t worker_cv;
  pthread_cond_t enque_cv; // cv for thread that add jobs to the buffer
  job *buffer;             // circular buffer that stores pointers to job
  int job_count;           // number of jobs in the buffer
  int cur_job;             // pointer to the job to execute
  int cur_fill;            // pointer in buffer where next job to be added
} tpool;

void *worker(void *arg) {
  for (;;) {
    // worker thread which will execute jobs from the queue
    tpool *tp = (tpool *)arg;
    int c;
    pthread_mutex_lock(&tp->mutex);
    while (tp->job_count == 0) {
      pthread_cond_wait(&tp->worker_cv, &tp->mutex);
    }
    c = tp->cur_job;
    tp->cur_job = (tp->cur_job + 1) % BUFFER_SIZE; // circular update of cur_job
    tp->job_count--;                               // decrement the number of jobs in the buffer
    pthread_cond_signal(&tp->enque_cv);
    pthread_mutex_unlock(&tp->mutex);
    (tp->buffer + c)->f((tp->buffer + c)->arg); // call job_func, pass arg parameter
  }
}

int tpool_add(tpool *tp, void *(*func)(void *), void *arg) {
  pthread_mutex_lock(&tp->mutex);
  while (tp->job_count >= BUFFER_SIZE) {
    pthread_cond_wait(&tp->enque_cv, &tp->mutex);
  }
  (tp->buffer + tp->cur_fill)->f = func;
  (tp->buffer + tp->cur_fill)->arg = arg;
  tp->job_count++;
  tp->cur_fill = (tp->cur_fill + 1) % BUFFER_SIZE;
  pthread_cond_signal(&tp->worker_cv);
  pthread_mutex_unlock(&tp->mutex);
  return 0;
}

tpool *tpool_create(int thread_count) {
  tpool *tp = malloc(sizeof(*tp));
  pthread_t *th = malloc(sizeof(*th) * thread_count);
  if (!tp || !th) {
    free(tp);
    free(th);
    ERR("tpool_create: couldn't allocate memory");
    return NULL;
  }
  int th_failure = 0;
  for (int i = 0; i < thread_count; i++) {
    int s = pthread_create(th + i, NULL, worker, (void *)tp);
    if (s != 0) {
      th_failure++;
      ERR("pthread_create: %s\n", strerror(s));
    }
  }
  /*All calls to pthread_create failed.*/
  if (th_failure >= thread_count) {
    free(tp);
    free(th);
    ERR("couldn't create a single thread.\n");
    return NULL;
  }
  /*`thread_count` should decrement if there are failure during pthread_create.*/
  tp->thread_count = thread_count - th_failure;
  tp->threads = th;
  int mutex_ch = pthread_mutex_init(&tp->mutex, NULL);
  int cond_ch1 = pthread_cond_init(&tp->worker_cv, NULL);
  int cond_ch2 = pthread_cond_init(&tp->enque_cv, NULL);
  job *buf = malloc(sizeof(job) * BUFFER_SIZE);
  if (mutex_ch != 0 || cond_ch1 != 0 || cond_ch2 != 0 || !buf) {
    free(tp);
    free(th);
    free(buf);
    pthread_cond_destroy(&tp->worker_cv);
    pthread_cond_destroy(&tp->enque_cv);
    ERR("tpool_create: couldn't initialize synchronization primitive/s");
    return NULL;
  }
  tp->buffer = buf;
  tp->job_count = 0;
  tp->cur_job = 0;
  tp->cur_fill = 0;
  return tp;
}

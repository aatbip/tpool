#include "tpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 50 // Should be able to declare during tpool_create?

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
  job *buffer;     // circular buffer that stores pointers to job
  int buffer_fill; // number of items in the buffer
  int cur_job;     // pointer to the job to execute
} tpool;

void *worker(void *arg) {
  // worker thread which will execute jobs from the queue
  tpool *tp = (tpool *)arg;
  pthread_mutex_lock(&tp->mutex);
  while (tp->buffer_fill == 0) {
    pthread_cond_wait(&tp->worker_cv, &tp->mutex);
  }

  pthread_mutex_unlock(&tp->mutex);

  return NULL;
}

tpool *tpool_create(int thread_count) {
  tpool *tp = malloc(sizeof(*tp));
  if (!tp)
    return NULL;
  pthread_t *th = malloc(sizeof(*th) * thread_count);
  if (!th) {
    free(tp);
    return NULL;
  }
  int th_failure = 0;
  for (int i = 0; i < thread_count; i++) {
    int s = pthread_create(th + i, NULL, worker, (void *)tp);
    if (s != 0) {
      th_failure++;
      fprintf(stderr, "pthread_create: %s\n", strerror(s));
    }
  }
  if (th_failure >= thread_count) {
    free(tp);
    free(th);
    return NULL;
  }
  /*`thread_count` should decrement if there failure during pthread_create.*/
  tp->thread_count = thread_count - th_failure;
  tp->threads = th;
  int s = pthread_mutex_init(&tp->mutex, NULL);
  if (s != 0) {
    free(tp);
    free(th);
    return NULL;
  }
  s = pthread_cond_init(&tp->worker_cv, NULL);
  job *buf = malloc(sizeof(job) * BUFFER_SIZE);
  if (s != 0 || !buf) {
    free(tp);
    free(th);
    return NULL;
  }
  tp->buffer = buf;
  tp->buffer_fill = 0;
  tp->cur_job = 0;
  return tp;
}

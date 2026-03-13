#include "tpool.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct _tpool {
  int thread_count;

} tpool;

void *worker(void *arg) {
  // worker thread which will execute jobs from the queue
  tpool *tp = (tpool *)arg;

  return NULL;
}

tpool *tpool_create(int thread_count) {
  tpool *tp = malloc(sizeof(*tp));
  tp->thread_count = thread_count;
  pthread_t *th = malloc(sizeof(*th) * thread_count);
  for (int i = 0; i < thread_count; i++) {
    pthread_create(th + i, NULL, worker, (void *)tp);
  }
  return tp;
}

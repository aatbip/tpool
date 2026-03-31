/*Test tpool_shutdown*/

#include "tpool.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int num = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void increment(void *arg) {
  pthread_mutex_lock(&mutex);
  num++;
  pthread_mutex_unlock(&mutex);
}

int main(void) {
  tpool *tp = tpool_create(4, 50);
  assert(tp != NULL);
  for (int i = 0; i < 1000; i++) {
    if (i == 500) {
      tpool_shutdown(tp);
    }
    tpool_add(tp, increment, NULL);
  }
  tpool_wait(tp);
  tpool_destroy(tp);

  printf("num: %d\n", num);
  assert(num == 500);

  return 0;
}

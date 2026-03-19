/*Tests goes here.*/
#include "tpool.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

void *job(void *arg) {
  printf("job %d\n", (int)(uintptr_t)arg);
  return NULL;
}

int main(void) {
  /*Create tpool with 4 worker threads.*/
  tpool *tp = tpool_create(4);
  assert(tp != NULL);

  for (int i = 0; i < 50; i++) {
    tpool_add(tp, job, (void *)(uintptr_t)i);
  }

  return 0;
}

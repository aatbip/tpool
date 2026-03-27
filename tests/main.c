/*Tests goes here.*/
#include "tpool.h"
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double get_time_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

void *job(void *arg) {
  printf("job: %d\n", (int)(uintptr_t)arg);
  return NULL;
}

// void *job(void *arg) {
//   int n = (int)(uintptr_t)arg + 1000000;
//   int count = 0;
//   for (int i = 2; i <= n; i++) {
//     int prime = 1;
//     for (int j = 2; j * j <= i; j++) {
//       if (i % j == 0) {
//         prime = 0;
//         break;
//       }
//     }
//     count += prime;
//   }
//   (void)count;
//   return NULL;
// }

int main(void) {
  /*Create tpool with 4 worker threads.*/
  double start = get_time_ms();
  tpool *tp = tpool_create(4);
  assert(tp != NULL);

  for (int i = 0; i < 4; i++) {
    tpool_add(tp, job, (void *)(uintptr_t)i);
  }
  tpool_wait(tp);
  tpool_destroy(tp);
  double end = get_time_ms();
  printf("tpool:Operation took %.3f ms\n", end - start);

  return 0;
}
// int main(void) {
//   double start = get_time_ms();
//   for (int i = 0; i < 500; i++) {
//     job((void *)(uintptr_t)i);
//   }
//   double end = get_time_ms();
//   printf("Operation took %.3f ms\n", end - start);
//   return 0;
// }

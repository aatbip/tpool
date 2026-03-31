/*Test if working thread count is returned correctly or not.*/
#include "tpool.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

void job(void *arg) {
  sleep(2);
  printf("Sleeping..\n");
}

int main(void) {
  tpool *tp = tpool_create(4, 50);
  for (int i = 0; i < 4; i++) { // Add 4 jobs
    tpool_add(tp, job, NULL);
  }
  sleep(1);
  int num = tpool_count_working_threads(tp);
  printf("%d\n", tpool_count_working_threads(tp));
  assert(num == 4);

  for (int i = 0; i < 2; i++) { // Add 2 jobs
    tpool_add(tp, job, NULL);
  }
  sleep(1);
  num = tpool_count_working_threads(tp);
  printf("%d\n", tpool_count_working_threads(tp));
  assert(num == 2);

  tpool_destroy(tp);
  return 0;
}

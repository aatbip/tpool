/*Test concurrent increment*/
#include <stdio.h>

int num = 0;

void *increment(void *arg) {
  num++;
  return NULL;
}

int main(void) {
  printf("running example 2");

  return 0;
}

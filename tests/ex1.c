/* This example uses tpool library to run the recursive calls of quick sort algorithm in a concurrent manner.
 * Benchmark will be added in readme.*/

#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Returns time in milliseconds
double get_time_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static inline void swap(int *a, int *b) {
  int tempA = *a;
  *a = *b;
  *b = tempA;
}

int median_of_three(int *arr, int left, int right) {
  int mid = left + (right - left) / 2;
  if (arr[left] > arr[mid]) {
    swap(&arr[left], &arr[mid]);
  }
  if (arr[left] > arr[right]) {
    swap(&arr[left], &arr[right]);
  }
  if (arr[mid] > arr[right]) {
    swap(&arr[mid], &arr[right]);
  }
  return mid;
}

int partition(int *nums, int left, int right, int pivot_index) {
  int pivot = nums[pivot_index];
  swap(&nums[pivot_index], &nums[right]);
  int p = left;
  for (int i = left; i < right; i++) {
    if (nums[i] < pivot) {
      swap(&nums[i], &nums[p]);
      p++;
    }
  }
  swap(&nums[p], &nums[right]);
  return p;
}

struct c {
  int *nums, n, left, right;
  tpool *tp;
};

int THRESSHOLD = 50;

void sort(int *nums, int n, int left, int right) {
  if (left >= right)
    return;
  // int pivot_index = rand() % (right - left + 1) + left;
  int pivot_index = median_of_three(nums, left, right);
  int p = partition(nums, left, right, pivot_index);
  sort(nums, n, p + 1, right);
  sort(nums, n, left, p - 1);
}

void sort_t(void *args) {
  struct c *a = (struct c *)args;
  if (a->left >= a->right) {
    free(a);
    return;
  }
  int pivot_index = median_of_three(a->nums, a->left, a->right);
  // int pivot_index = rand() % (a->right - a->left + 1) + a->left;
  THRESSHOLD--;
  int p = partition(a->nums, a->left, a->right, pivot_index);
  if (THRESSHOLD <= 0) {
    sort(a->nums, a->n, a->left, p - 1);
    sort(a->nums, a->n, p + 1, a->right);
  } else {
    if (a->left < p - 1) {
      struct c *r1 = malloc(sizeof(struct c));
      r1->nums = a->nums;
      r1->n = a->n;
      r1->left = a->left;
      r1->right = p - 1;
      r1->tp = a->tp;
      tpool_add(a->tp, sort_t, (void *)r1);
    }
    if (p + 1 < a->right) {
      struct c *r2 = malloc(sizeof(struct c));
      r2->nums = a->nums;
      r2->n = a->n;
      r2->left = p + 1;
      r2->right = a->right;
      r2->tp = a->tp;
      tpool_add(a->tp, sort_t, (void *)r2);
    }
  }
  free(a);
}

void quicksort(int *nums, int n, tpool *tp) {
  srand(time(NULL)); // seeding to make sure rand() generates variable sequence
  if (tp == NULL) {
    sort(nums, n, 0, n - 1);
  } else {
    struct c *a = malloc(sizeof(struct c));
    a->nums = nums;
    a->n = n;
    a->left = 0;
    a->right = n - 1;
    a->tp = tp;
    sort_t((void *)a);
  }
}

int main(void) {
  int s = 999999999;
  int *nums = malloc(sizeof(int) * s);

  for (int i = 0; i < s; i++) {
    nums[i] = s - i;
  }

  printf("before sort nums[first]: %d\n", nums[0]);
  printf("before sort nums[last]: %d\n", nums[s - 1]);

  double start = get_time_ms();

  /*Uncomment this to run without tpool*/
  // quicksort(nums, s, NULL);

  /*Uncomment this to run concurrently using tpool*/
  tpool *tp = tpool_create(4, THRESSHOLD);
  quicksort(nums, s, tp);
  tpool_wait(tp);
  tpool_destroy(tp);

  double end = get_time_ms();
  printf("Operation took %.3f ms\n", end - start);

  printf("after sort nums[first] %d\n", nums[0]);
  printf("after sort nums[last] %d\n", nums[s - 1]);
  free(nums);

  return 0;
}

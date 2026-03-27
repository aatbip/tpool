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

void sort(int *nums, int n, int left, int right) {
  if (left >= right)
    return;
  // int pivot_index = rand() % (right - left + 1) + left;
  int pivot_index = median_of_three(nums, left, right);
  int p = partition(nums, left, right, pivot_index);
  sort(nums, n, p + 1, right);
  sort(nums, n, left, p - 1);
}

void quicksort(int *nums, int n) { sort(nums, n, 0, n - 1); }

struct argument {
  int *nums;
  int n;
};

void *job1(void *x) {
  struct argument *arg = (struct argument *)x;
  printf("yoo: %d\n", arg->n);
  quicksort(arg->nums, arg->n);
  return NULL;
}
void *fill(void *x) {
  printf("filling...\n");
  int *nums = (void *)x;
  for (int i = 0; i < 1000000000; i++) {
    nums[i] = rand();
  }
  return NULL;
}

int main(void) {
  /*Create tpool with 4 worker threads.*/
  double start = get_time_ms();
  tpool *tp = tpool_create(6);
  assert(tp != NULL);
  int *nums1 = malloc(sizeof(int) * 1000000000);
  int *nums2 = malloc(sizeof(int) * 1000000000);
  int *nums3 = malloc(sizeof(int) * 1000000000);
  int *nums4 = malloc(sizeof(int) * 1000000000);
  srand(time(NULL)); // seeding to make sure rand() generates variable sequence
  tpool *tp1 = tpool_create(6);
  tpool_add(tp1, fill, nums1);
  tpool_add(tp1, fill, nums2);
  tpool_add(tp1, fill, nums3);
  tpool_add(tp1, fill, nums4);
  tpool_wait(tp1);
  tpool_destroy(tp1);
  struct argument arg1 = {.nums = nums1, .n = 1000000000};
  struct argument arg2 = {.nums = nums2, .n = 1000000000};
  struct argument arg3 = {.nums = nums3, .n = 1000000000};
  struct argument arg4 = {.nums = nums4, .n = 1000000000};
  struct argument args[] = {arg1, arg2, arg3, arg4};
  for (int i = 0; i < 4; i++) {
    tpool_add(tp, job1, (void *)&args[i]);
  }
  tpool_wait(tp);
  tpool_destroy(tp);
  double end = get_time_ms();
  printf("tpool:Operation took %.3f ms\n", end - start);
  for (int i = 0; i < 20; i++) {
    printf("%d ", nums1[i]);
  }

  return 0;
}

// int main(void) {
//   double start = get_time_ms();
//   int *nums1 = malloc(sizeof(int) * 1000000000);
//   int *nums2 = malloc(sizeof(int) * 1000000000);
//   int *nums3 = malloc(sizeof(int) * 1000000000);
//   int *nums4 = malloc(sizeof(int) * 1000000000);
//   srand(time(NULL)); // seeding to make sure rand() generates variable sequence
//   for (int i = 0; i < 1000000000; i++) {
//     nums1[i] = rand();
//   }
//   for (int i = 0; i < 1000000000; i++) {
//     nums2[i] = rand();
//   }
//   for (int i = 0; i < 1000000000; i++) {
//     nums3[i] = rand();
//   }
//   for (int i = 0; i < 1000000000; i++) {
//     nums4[i] = rand();
//   }
//   struct argument arg1 = {.nums = nums1, .n = 1000000000};
//   struct argument arg2 = {.nums = nums2, .n = 1000000000};
//   struct argument arg3 = {.nums = nums3, .n = 1000000000};
//   struct argument arg4 = {.nums = nums4, .n = 1000000000};
//   struct argument args[] = {arg1, arg2, arg3, arg4};
//   for (int i = 0; i < 4; i++) {
//     job1((void *)&args[i]);
//   }
//   double end = get_time_ms();
//   printf("tpool:Operation took %.3f ms\n", end - start);
//   for (int i = 0; i < 20; i++) {
//     printf("%d ", nums1[i]);
//   }
//
//   return 0;
// }

#include "tpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void *sort_t(void *args) {
  struct c *a = (struct c *)args;
  if (a->left >= a->right)
    return NULL;
  // int pivot_index = median_of_three(a->nums, a->left, a->right);
  int pivot_index = rand() % (a->right - a->left + 1) + a->left;
  int p = partition(a->nums, a->left, a->right, pivot_index);
  struct c r1 = {.nums = a->nums, .n = a->n, .left = p + 1, .right = a->right};
  tpool_add(a->tp, sort_t, (void *)&r1);
  struct c r2 = {.nums = a->nums, .n = a->n, .left = a->left, .right = p - 1};
  tpool_add(a->tp, sort_t, (void *)&r2);
  return NULL;
}

// void sort(int *nums, int n, int left, int right) {
//   if (left >= right)
//     return;
//   // int pivot_index = rand() % (right - left + 1) + left;
//   int pivot_index = median_of_three(nums, left, right);
//   int p = partition(nums, left, right, pivot_index);
//   sort(nums, n, p + 1, right);
//   sort(nums, n, left, p - 1);
// }

void quicksort(int *nums, int n, tpool *tp) {
  srand(time(NULL)); // seeding to make sure rand() generates variable sequence
  struct c a = {.nums = nums, .n = n, .left = 0, .right = n - 1, .tp = tp};
  sort_t((void *)&a);
}

int main(void) {
  tpool *tp = tpool_create(2);
  int nums[] = {6, 2, 4, 1, 3, 0};
  quicksort(nums, 6, tp);
  tpool_wait(tp);
  tpool_destroy(tp);

  for (int i = 0; i < 6; i++) {
    printf("%d ", nums[i]);
  }

  return 0;
}

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
  if (a->left >= a->right) {
    free(a);
    return NULL;
  }
  int pivot_index = median_of_three(a->nums, a->left, a->right);
  // int pivot_index = rand() % (a->right - a->left + 1) + a->left;
  int p = partition(a->nums, a->left, a->right, pivot_index);
  struct c *r1 = malloc(sizeof(struct c));
  r1->nums = a->nums;
  r1->n = a->n;
  r1->left = p + 1;
  r1->right = a->right;
  r1->tp = a->tp;
  tpool_add(a->tp, sort_t, (void *)r1);
  struct c *r2 = malloc(sizeof(struct c));
  r2->nums = a->nums;
  r2->n = a->n;
  r2->left = a->left;
  r2->right = p - 1;
  r2->tp = a->tp;
  tpool_add(a->tp, sort_t, (void *)r2);
  free(a);
  return NULL;
}

void quicksort(int *nums, int n, tpool *tp) {
  srand(time(NULL)); // seeding to make sure rand() generates variable sequence
  struct c *a = malloc(sizeof(struct c));
  a->nums = nums;
  a->n = n;
  a->left = 0;
  a->right = n - 1;
  a->tp = tp;
  sort_t((void *)a);
}

int main(void) {
  int s = 500;
  int *nums = malloc(sizeof(int) * s);

  for (int i = 0; i < s; i++) {
    nums[i] = s - i;
  }

  printf("before sort nums[first]: %d\n", nums[0]);
  printf("before sort nums[last]: %d\n", nums[s - 1]);

  tpool *tp = tpool_create(2);
  quicksort(nums, s, tp);
  tpool_wait(tp);
  tpool_destroy(tp);

  printf("after sort nums[first] %d\n", nums[0]);
  printf("after sort nums[last] %d\n", nums[s - 1]);
  free(nums);

  return 0;
}

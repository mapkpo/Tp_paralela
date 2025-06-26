/*
 * mergesort.c
 *
 * CS 470 Project 3 (MPI)
 * Original serial version.
 *
 * Compile with --std=c99
 */

// A este implementarlo con MPI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// histogram bins
#define BINS 10

// maximum random number
#define RMAX 100

// enable debug output
/*#define DEBUG*/

// "count_t" used for number counts that could become quite high
typedef unsigned long count_t;

int *nums;            // random numbers
count_t *hist;        // histogram (counts of "nums" in bins)
count_t  global_n;    // global "nums" count

/*
 * Compares two ints. Suitable for calls to standard qsort routine.
 */
int cmp(const void* a, const void* b)
{
    return *(int*)a - *(int*)b;
}

/*
 * Print contents of an int list.
 */
void print_nums(int *a, count_t n)
{
    for (count_t i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

/*
 * Print contents of a count list (i.e., histogram).
 */
void print_counts(count_t *a, count_t n)
{
    for (count_t i = 0; i < n; i++) {
        printf("%lu ", a[i]);
    }
    printf("\n");
}

/*
 * Merge two sorted lists ("left" and "right) into "dest" using temp storage.
 */
void merge(int left[], count_t lsize, int right[], count_t rsize, int dest[])
{
    count_t dsize = lsize + rsize;
    int *tmp = (int*)malloc(sizeof(int) * dsize);
    count_t l = 0, r = 0;
    for (count_t ti = 0; ti < dsize; ti++) {
        if (l < lsize && (left[l] <= right[r] || r >= rsize)) {
            tmp[ti] = left[l++];
        } else {
            tmp[ti] = right[r++];
        }
    }
    memcpy(dest, tmp, dsize*sizeof(int));
    free(tmp);
}

/*
 * Generate random integers for "nums".
 */
void randomize()
{
    srand(42);
    for (count_t i = 0; i < global_n; i++) {
        nums[i] = rand() % RMAX;
    }
#   ifdef DEBUG
    printf("global orig list: "); print_nums(nums, global_n);
#   endif
}

/*
 * Calculate histogram based on contents of "nums".
 */
void histogram()
{
    for (count_t i = 0; i < global_n; i++) {
        hist[nums[i] % BINS]++;
    }
}

/*
 * Merge sort helper (shouldn't be necessary in parallel version).
 */
void merge_sort_helper(int *start, count_t len)
{
    if (len < 100) {
        qsort(start, len, sizeof(int), cmp);
    } else {
        count_t mid = len/2;
        merge_sort_helper(start, mid);
        merge_sort_helper(start+mid, len-mid);
        merge(start, mid, start+mid, len-mid, start);
    }
}

/*
 * Sort "nums" using the mergesort algorithm.
 */
void merge_sort()
{
    merge_sort_helper(nums, global_n);
}

int main(int argc, char *argv[])
{
    // read command-line parameters
    if (argc != 2) {
        printf("Usage: %s <n>\n", argv[0]);
        global_n = -1;
    } else {
        global_n = strtol(argv[1], NULL, 10);
    }

    // exit if command-line parameters were bad
    if (global_n == -1) {
        exit(EXIT_FAILURE);
    }

    // initialize local data structures
    nums = (int*)malloc(sizeof(int) * global_n);
    hist = (count_t*)malloc(sizeof(count_t) * BINS);
    memset(nums, 0, sizeof(int) * global_n);
    memset(hist, 0, sizeof(count_t) * BINS);

    // initialize random numbers
    clock_t rand_time = clock();
    randomize();
    rand_time = clock() - rand_time;

    // compute histogram
    clock_t hist_time = clock();
    histogram();
    hist_time = clock() - hist_time;

    // perform merge sort
    clock_t sort_time = clock();
    merge_sort();
    sort_time = clock() - sort_time;

    // print global results
    printf("GLOBAL hist: "); print_counts(hist, BINS);
#   ifdef DEBUG
    printf("GLOBAL list: "); print_nums(nums, global_n);
#   endif
    printf("RAND: %.4f  HIST: %.4f  SORT: %.4f\n",
            (float)rand_time/(float)CLOCKS_PER_SEC,
            (float)hist_time/(float)CLOCKS_PER_SEC,
            (float)sort_time/(float)CLOCKS_PER_SEC);

    // clean up and exit
    free(nums);
    free(hist);
    return EXIT_SUCCESS;
}
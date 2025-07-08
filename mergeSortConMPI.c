#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h> 

// histogram bins
#define BINS 10

// maximum random number
#define RMAX 100

#define MASTER 0

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
 * Calculate histogram based on contents of the array. Each node executes locally
 */
void histogram(int *array, int n, count_t *hist) {
    for (int i = 0; i < n; ++i) {
        hist[array[i] % BINS]++;
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

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, numtasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    if (argc != 2) {
        if (rank == MASTER)
            printf("Usage: %s <n>\n", argv[0]);
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    global_n = strtol(argv[1], NULL, 10);
    if (global_n <= 0) {
        if (rank == MASTER)
            printf("Invalid number count.\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int base = global_n / numtasks;
    int extra = global_n % numtasks;
    int local_n = base + (rank < extra ? 1 : 0);

    int *sendcounts = NULL, *displs = NULL;
    if (rank == MASTER) {
        nums = malloc(sizeof(int) * global_n);
        sendcounts = malloc(sizeof(int) * numtasks);
        displs = malloc(sizeof(int) * numtasks);

        int offset = 0;
        for (int i = 0; i < numtasks; i++) {
            sendcounts[i] = base + (i < extra ? 1 : 0);
            displs[i] = offset;
            offset += sendcounts[i];
        }

        clock_t rand_time = clock();
        randomize();
        rand_time = clock() - rand_time;
        printf("RAND: %.4f  ", (float)rand_time / CLOCKS_PER_SEC);
    }

    int *local_nums = malloc(sizeof(int) * local_n);
    count_t *local_hist = calloc(BINS, sizeof(count_t));

    MPI_Scatterv(nums, sendcounts, displs, MPI_INT,
                 local_nums, local_n, MPI_INT, MASTER, MPI_COMM_WORLD);

    clock_t hist_time = clock();
    histogram(local_nums, local_n, local_hist);
    hist_time = clock() - hist_time;

    if (rank == MASTER)
        hist = calloc(BINS, sizeof(count_t));

    MPI_Reduce(local_hist, hist, BINS, MPI_UNSIGNED_LONG,
               MPI_SUM, MASTER, MPI_COMM_WORLD);

    clock_t sort_time = clock();
    merge_sort_helper(local_nums, local_n);
    sort_time = clock() - sort_time;

    int *recvbuf = NULL;
    if (rank == MASTER)
        recvbuf = malloc(sizeof(int) * global_n);

    MPI_Gatherv(local_nums, local_n, MPI_INT,
                recvbuf, sendcounts, displs, MPI_INT, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        for (int step = 1; step < numtasks; step *= 2) {
            for (int i = 0; i + step < numtasks; i += 2 * step) {
                int l_start = displs[i];
                int l_size  = sendcounts[i];
                int r_start = displs[i + step];
                int r_size  = sendcounts[i + step];
                merge(&recvbuf[l_start], l_size,
                      &recvbuf[r_start], r_size,
                      &recvbuf[l_start]);
                sendcounts[i] += sendcounts[i + step];
            }
        }

        printf("HIST: %.4f  SORT: %.4f\n",
               (float)hist_time / CLOCKS_PER_SEC,
               (float)sort_time / CLOCKS_PER_SEC);

        printf("GLOBAL hist: ");
        print_counts(hist, BINS);
    }

    // liberar memoria
    free(local_nums);
    free(local_hist);
    if (rank == MASTER) {
        free(nums);
        free(hist);
        free(sendcounts);
        free(displs);
        free(recvbuf);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

/*
Muhammad Huzaifa
20I-0604
Parallel & Distributed Computing
Assignment # 01 Task # 01
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define NUM_THREADS 4

int matrix[MATRIX_SIZE][MATRIX_SIZE];
pthread_t threads[NUM_THREADS];
long distsum = 0;

struct Thread_Args
{
    int rows_per_thread;
    int **buffer_array;
    int threadid;
    long *thread_sum; // Store the partial sum for each thread
};

void Initialize_array()
{
    srand(time(NULL));
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matrix[i][j] = rand() % 1024;
        }
    }
}

long Original_Sum()
{
    long original = 0;
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            original += matrix[i][j];
        }
    }
    return original;
}

void *calculateColSum(void *object)
{
    printf("hello in");
    struct Thread_Args *structure = (struct Thread_Args *)object;
    long temp_sum = 0; // Local variable to store the sum

    for (int i = 0; i < structure->rows_per_thread; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            temp_sum += structure->buffer_array[i][j];
        }
    }

    *(structure->thread_sum) = temp_sum;
    printf("\n \033[1;34m [-] \033[0m Printing Sum of rows: \033[33m%d\033[0m\n\n", temp_sum);
    return NULL;
}

int **create_buffer(int rows, int start, int end)
{
    int **buffer = (int **)malloc(rows * sizeof(int *));
    for (int i = start; i < end; i++)
    {
        buffer[i-start] = (int *)malloc(MATRIX_SIZE * sizeof(int));
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            buffer[i-start][j] = matrix[i][j];
        }
    }
    return buffer;
}


int main()
{
    Initialize_array();
    //long long sum = 0;
    printf("hello1");
    struct Thread_Args thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        printf("hello %d", i);
        int rows_per_thread = MATRIX_SIZE / NUM_THREADS;
        int start_row = i * rows_per_thread;
        int end_row = (i == NUM_THREADS - 1) ? MATRIX_SIZE : (start_row + rows_per_thread);

        thread_args[i].rows_per_thread = rows_per_thread;
        thread_args[i].threadid = i;
        thread_args[i].thread_sum = (long *)malloc(sizeof(long)); // Allocate memory for partial sum
        *(thread_args[i].thread_sum) = 0; // Initialize partial sum to 0
        thread_args[i].buffer_array = create_buffer(rows_per_thread, start_row, end_row);

        pthread_create(&threads[i], NULL, calculateColSum, &thread_args[i]);
    }
    printf("hello out");
    distsum = 0; // Initialize distsum to zero
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        distsum += *(thread_args[i].thread_sum); // Accumulate the partial sums
        free(thread_args[i].thread_sum); // Free memory allocated for partial sum
    }

    long Actual_Sum = Original_Sum();

    if (Actual_Sum == distsum)
    {
        printf("\n\t\t\t \033[32m ===============  Threads working fine both sums are equal. =============== \033[0m\n\n");
        printf("\t\t\t\t\tSum of Original Array: %ld\n", Actual_Sum);
        printf("\t\t\t\t\tSum of C-Sliced Array: %ld\n", distsum);
    }
    else
    {
        printf("\n\t\t\t \033[31m =============== Threads working fine but computation mistake. =============== \033[0m \n\n");
        printf("\t\t\t\t\tSum of Original Array: %ld \n", Actual_Sum);
        printf("\t\t\t\t\tSum of C-Sliced Array: %ld \n", distsum);
    }

    return 0;
}
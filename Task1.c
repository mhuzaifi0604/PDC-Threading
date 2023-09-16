/*
Muhammad Huzaifa
20I-0604
Parallel & Distributed Computing
Assignment # 01 Task # 01
*/

// Including Necessory Libraries
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Defining global constant variables
#define MATRIX_SIZE 1024
#define NUM_THREADS 4

// Defining Global arrays
int matrix[MATRIX_SIZE][MATRIX_SIZE];
pthread_t threads[NUM_THREADS];
long distsum = 0;

// Creating a struct for buffer arrays and distributed sums
struct Thread_Args
{
    int rows_per_thread;
    int **buffer_array;
    int threadid;
    long *thread_sum; // Store the partial sum for each thread
};

// Function for initializing original array randomly
void Initialize_array()
{
    srand(time(NULL));
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matrix[i][j] = rand() % 1024; // Taking mod with 1024 for better randomization
        }
    }
}

// function to calculate sum of original array
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

// function for each thread to calculate portion sum of original array
void *calculateRowSum(void *object)
{
    // type casting structure object from void * to struct *
    struct Thread_Args *structure = (struct Thread_Args *)object;
    long temp_sum = 0; // Local variable to store the sum

    // Oouter loop = rows in a sub array since it is row wise distribution
    for (int i = 0; i < structure->rows_per_thread; i++)
    {
        // Each row has 1024 cols hence inner loop runs untill Matrix_Size
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            temp_sum += structure->buffer_array[i][j];// Adding sum of array to local variable
        }
    }
    // Adding local portion sum into objects thread sum due to thread saftey and accurate sum at the end
    *(structure->thread_sum) = temp_sum;
    printf("\n \033[1;34m [-] \033[0m Printing Sum of rows: \033[33m%d\033[0m\n\n", temp_sum); // Printing Local portion Sum
    return NULL;
}

// functin to copy original array into buffer 
int **create_buffer(int rows, int start, int end)
{
    // Creating a 2D dynamic array having rows = rows_per_thread
    int **buffer = (int **)malloc(rows * sizeof(int *));
    for (int i = start; i < end; i++)
    {
        // each portion array will have cols = 1024
        buffer[i-start] = (int *)malloc(MATRIX_SIZE * sizeof(int));
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            buffer[i-start][j] = matrix[i][j]; // copying respective portion from original array into a buffer
        }
    }
    return buffer; // Returning buffer to main 
}


int main()
{
    // Initializing original array randomly
    Initialize_array();
    // Creating an array of objects for threads
    struct Thread_Args thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        // number of rows for each subarray --> 1024 / 4 == 256
        int rows_per_thread = MATRIX_SIZE / NUM_THREADS;
        // getting start of row [ 0 * 256 == 0, 1 * 256 == 256 , so on]
        int start_row = i * rows_per_thread;
        int end_row = (i == NUM_THREADS - 1) ? MATRIX_SIZE : (start_row + rows_per_thread); // calculating end of row for each subarray

        // Assigning values to respective variables in thread object
        thread_args[i].rows_per_thread = rows_per_thread;
        thread_args[i].threadid = i;
        thread_args[i].thread_sum = (long *)malloc(sizeof(long)); // Allocate memory for partial sum
        *(thread_args[i].thread_sum) = 0; // Initialize partial sum to 0
        thread_args[i].buffer_array = create_buffer(rows_per_thread, start_row, end_row);
        // creating a thread and passing the respective object from array to calculate sum of portion array
        pthread_create(&threads[i], NULL, calculateRowSum, &thread_args[i]);
    }

    distsum = 0; // Initialize distsum to zero
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        distsum += *(thread_args[i].thread_sum); // Accumulate the partial sums
        free(thread_args[i].thread_sum); // Free memory allocated for partial sum
    }

    // getting sum of original array
    long Actual_Sum = Original_Sum();

    // Comparing original and distributed Sums
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
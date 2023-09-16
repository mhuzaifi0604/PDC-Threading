/*
Muhammad Huzaifa
20I-0604
Parallel & Distributed Computing
Assignment # 01 Task # 03
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Defining global constant variables
#define MATRIX_SIZE 1024
#define NUM_THREADS 4
#define Block 512

// Defining Global arrays
int matrix[MATRIX_SIZE][MATRIX_SIZE];
pthread_t threads[NUM_THREADS];
long distsum = 0;

// Creating a struct for buffer arrays and distributed sums
struct Thread_Args
{
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

// function for each thread to calculate Block of original array
void *calculateBlockSum(void *object)
{
    // type casting structure object from void * to struct *
    struct Thread_Args *structure = (struct Thread_Args *)object;
    long temp_sum = 0; // Local variable to store the sum

    // Since a block wise distribution outer and inner loop runs 512 times
    for (int i = 0; i < Block; i++)
    {
        for (int j = 0; j < Block; j++)
        {
            temp_sum += structure->buffer_array[i][j]; // adding buffer array's sum to lacal variable
        }
    }

    // Adding local portion sum into objects thread sum due to thread saftey and accurate sum at the end
    *(structure->thread_sum) = temp_sum;
    printf("\n \033[1;34m [-] \033[0m Printing Sum of rows: \033[33m%d\033[0m\n\n", temp_sum); // Printing Local portion Sum
    return NULL;
}

// functin to copy original array into buffer 
int **create_buffer(int start_row, int end_row, int start_col, int end_col)
{
    // creating a2D array having rows == 512 for each block
    int **buffer = (int **)malloc((end_row - start_row) * sizeof(int *));
    for (int i = start_row; i < end_row; i++)
    {
        // cols == 512 since block distribution of each block of 512 x 512
        buffer[i-start_row] = (int *)malloc((end_col - start_col) * sizeof(int));
        for (int j = start_col; j < end_col; j++)
        {
            buffer[i-start_row][j-start_col] = matrix[i][j]; // copying respective portion from org array to buffer
        }
    }
    return buffer; // returning buffer to main
}


int main()
{
    // Initializing original array randomly
    Initialize_array();
    int count = 0; // count for each thread for sending start and ends of rows and cols and thread creation
    // Creating an array of objects for threads
    struct Thread_Args thread_args[NUM_THREADS];

    // c raeting a nested loop | only need 4 iterations hence dividing Num_threads / 2
    for (int i = 0; i < NUM_THREADS/2; i++)
    {
        for (int j = 0; j < NUM_THREADS/2; j++){
            // Calculating start & end of row for each thread
            int start_row = j*Block;
            int end_row = (j==(NUM_THREADS/2) - 1 ) ? MATRIX_SIZE : (j+1)*Block;
            // Calculating start and end of col for each thread
            int start_col = i*Block;
            int end_col = (i==(NUM_THREADS/2)-1) ? MATRIX_SIZE : (i+1) * Block;
            // Assigning respective variables defined variables in structure above
            thread_args[count].threadid = count;
            thread_args[count].thread_sum = (long *)malloc(sizeof(long));
            *(thread_args[count].thread_sum) = 0;
            thread_args[count].buffer_array = create_buffer(start_row, end_row, start_col, end_col);
            // creating a thread and passing the respective object from array to calculate sum of portion array
            pthread_create(&threads[count], NULL, calculateBlockSum, &thread_args[count]);
            count += 1;
        }
    }
    distsum = 0; // Initialize distsum to zero
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
        distsum += *(thread_args[i].thread_sum); // Accumulate the partial sums
        free(thread_args[i].thread_sum); // Free memory allocated for partial sum
    }
    // getting sum of org array
    long Actual_Sum = Original_Sum();
    // Comparing original and distributed Sums
    if (Actual_Sum == distsum)
    {
        printf("\n\t\t\t \033[32m ===============  Threads working fine both sums are equal. =============== \033[0m\n\n");
        printf("\t\t\t\t\tSum of Original Array: %ld\n", Actual_Sum);
        printf("\t\t\t\t\tSum of B-Sliced Array: %ld\n", distsum);
    }
    else
    {
        printf("\n\t\t\t \033[31m =============== Threads working fine but computation mistake. =============== \033[0m \n\n");
        printf("\t\t\t\t\tSum of Original Array: %ld \n", Actual_Sum);
        printf("\t\t\t\t\tSum of B-Sliced Array: %ld \n", distsum);
    }

    return 0;
}
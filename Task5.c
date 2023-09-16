/*
Muhammad Huzaifa
20I-0604
Parallel & Distributed Computing
Assignment # 01 Task # 05
*/

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
    int buffer_array[MATRIX_SIZE];
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
void *calculateCyclicColSum(void *object)
{
    // type casting structure object from void * to struct *
    struct Thread_Args *structure = (struct Thread_Args *)object;
    long temp_sum = 0; // Local variable to store the sum
    //Calculating Sum of the col passed to thread
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
            temp_sum += structure->buffer_array[i]; // adding sum of passed col to local variable
    }

    // Adding local sum into objects thread sum due to thread saftey and accurate sum at the end
    *(structure->thread_sum) += temp_sum;
    printf("\n \033[1;34m [-] \033[0m Printing Sum of Cols:  \033[33m%d\033[0m\n\n", temp_sum); // Printing Local col Sum
    return NULL;
}


int main()
{
    // Initializing original array randomly
    Initialize_array();
    int buffer [MATRIX_SIZE]; // array for storing each col of original array and then passing it to thread function.
    // Creating an array of objects for threads
    struct Thread_Args thread_args[NUM_THREADS];
    int num = 0;
    distsum = 0; // Initialize distsum to zero

    // Loop for Cyclic distribution of cols among 4 Threads 
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        if(num == 4){ // setting thread number back to 0 if thread exceeds 4 due to 1024 loop size
            num =0;
        }
        // loop for copying each col of original array into a buffer 
        for(int j = 0; j< 1024; j++){
        thread_args[num].buffer_array[j] = matrix[j][i];
        }
        // Dynamically allocating thread_sum since it will get type casted into void *
        thread_args[num].thread_sum = (long *)malloc(sizeof(long));
        *(thread_args[num].thread_sum) = 0;
        // creating a thread and passing the respective object from array to calculate sum of col
        pthread_create(&threads[num], NULL, calculateCyclicColSum, &thread_args[num]);
        pthread_join(threads[num], NULL); // Joining threads in the loop too
        distsum += *(thread_args[num].thread_sum); // Accumulate the added col sums
        free(thread_args[num].thread_sum); // Free memory allocated for partial sum
        num += 1;
    }
    // getting original sum of array
    long Actual_Sum = Original_Sum();
    // Comparing original and distributed Sums
    if (Actual_Sum == distsum)
    {
        printf("\n\t\t\t \033[32m ===============  Threads working fine both sums are equal. =============== \033[0m\n\n");
        printf("\t\t\t\t\tSum of Original  Array: %ld\n", Actual_Sum);
        printf("\t\t\t\t\tSum of CC-Sliced Array: %ld\n", distsum);
    }
    else
    {
        printf("\n\t\t\t \033[31m =============== Threads working fine but computation mistake. =============== \033[0m \n\n");
        printf("\t\t\t\t\tSum of Original  Array: %ld \n", Actual_Sum);
        printf("\t\t\t\t\tSum of CC-Sliced Array: %ld \n", distsum);
    }

    return 0;
}
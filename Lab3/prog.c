#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>
#include <time.h>
typedef struct 
{
    char* matrix;
    unsigned long long prime_num;
    unsigned long long max_num;
    unsigned long long* count_threads;
} pthrData;
void* threadFunc(void* thread_data)
{
    pthrData *data = (pthrData*) thread_data;
    for (unsigned long long i = data->prime_num*data->prime_num; i <= data->max_num && i > data->prime_num; i += data->prime_num)
    {
        data->matrix[i] = 1;
    }
    *data->count_threads = *data->count_threads - 1;
    return NULL;
}
const int MAXIMUM_THREADS = 30;
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        perror("Error! The number of threads to create is not specified!\n");
        return -1;
    }
    unsigned long long count = atoi(argv[1]);
    if (count > MAXIMUM_THREADS)
    {
        perror("Error! Big count of threads!\n");
        return -1;
    }
    printf("Input numbrer to check: ");
    unsigned long long number;
    scanf("%llu", &number);
    char* matrix = NULL;
    matrix = (char*) malloc((number+1)*sizeof(char));
    if (matrix == NULL) 
    {
        perror("Memory allocation error!\n");
        return -2;
    }
    for (int i = 0; i <= number; ++i)
    {
        matrix[i] = 0;
    }
    pthread_t* threads = NULL;
    pthrData* threadsData = NULL;
    threads = (pthread_t*) malloc(count*sizeof(pthread_t));
    threadsData = (pthrData*) malloc(count*sizeof(pthrData));
    if (threads == NULL || threadsData == NULL) 
    { 
        perror("Memory allocation error!\n");
        return -2; 
    }
    unsigned long long count_threads = 0;
    long double start, finish;
    start = clock();
    for (unsigned long long i = 2; i*i < number && i*i > i; ++i)
    {
        if (matrix[i] == 0)
        {
	        if (count_threads < count)
	        {
	            int now_thread = count_threads;
	            matrix[i*i] = 2+now_thread;
	            threadsData[now_thread].matrix = matrix;
	            threadsData[now_thread].prime_num = i;
	            threadsData[now_thread].max_num = number;
	            threadsData[now_thread].count_threads = &count_threads;
	            if (pthread_create(&threads[now_thread], NULL, threadFunc, &threadsData[now_thread]))
	            {
	                perror("Error creating thread!\n");
	                return -4;
	            }	
	            ++count_threads;
	        }
	        else
	        {
		        for (unsigned long long j = i*i; j <= number && j > i; j += i)
		        {
		            matrix[j] = 1;
		        }
	        }
         }
         else if (matrix[i] >= 2)
         {
             if (pthread_join(threads[matrix[i]-2], NULL))
             {
                 perror("Error executing thread!\n");
                 return -6;
             }
         }
    }
    for (int i = 0; i < count_threads; ++i)
    {
        if (pthread_join(threads[i], NULL))
        {
            perror("Error execution thread!\n");
            return -6;
        }
    }
    finish = clock();
    printf("Time of execution %Lf ms.\n", (finish - start)/1000.0);
    if (matrix[number])
    {
        printf("%llu is composite number.\n", number);
    }
    else
    {
        printf("%llu is prime number.\n", number);
    }
    free(threads);
    free(threadsData);
    free(matrix);
    return 0;
}

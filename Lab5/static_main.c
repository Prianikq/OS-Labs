#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int comand;
    while (scanf("%d", &comand) != EOF)
    {
        if (comand == 1)
        {
            int k;
            scanf("%d", &k);
            float value = Pi(k);
            printf("Pi(%d) = %f\n",  k, value);
        }
        else if (comand == 2)
        {
            int n;
            scanf("%d", &n);
            int* array = NULL;
            array = malloc(n * sizeof(int));
            if (array == NULL)
            {
                printf("Error allocating memory!\n");
                return -1;
            }
            for (int i = 0; i < n; ++i)
            {
                scanf("%d", &array[i]);
            }
            array = Sort(array, n);
            printf("Sort(array) = [ ");
            for (int i = 0; i < n; ++i)
            {
                printf("%d", array[i]);
                if (i != n - 1) printf(", ");
            }
            printf(" ]\n");
            free(array);
        }
        else
        {
        	printf("Error! Wrong input!\n");
        }
    }
    return 0;
}

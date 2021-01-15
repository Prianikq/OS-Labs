#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#define check_ok(VAR, VAL) if (VAR != VAL) { printf("Error: %s\n", dlerror()); return -1; }
#define check_wrong(VAR, VAL) if (VAR == VAL) { printf("Error: %s\n", dlerror()); return -1; }
const char* LIBRARY1_NAME = "./realization1.so";
const char* LIBRARY2_NAME = "./realization2.so";
const char* FUNCTION1_NAME = "Pi";
const char* FUNCTION2_NAME = "Sort";
int main()
{
    void* dl_handler = dlopen(LIBRARY1_NAME, RTLD_LAZY);
    check_wrong(dl_handler, NULL);
    int type = 1;
    int command;
    while (scanf("%d", &command) != EOF)
    {
        if (command == 0)
        {
        	check_ok(dlclose(dl_handler), 0);
            dl_handler = (type == 1? dlopen(LIBRARY2_NAME, RTLD_LAZY) : dlopen(LIBRARY1_NAME, RTLD_LAZY));
            check_wrong(dl_handler, NULL);
            type = (type == 1? 2 : 1);
            printf("Change dynamic library from %d to %d\n", (type == 1? 2 : 1), type);
        }
        else if (command == 1)
        {
            float (*Pi)(int) = dlsym(dl_handler, FUNCTION1_NAME);
            check_wrong(Pi, NULL);
            int k;
            scanf("%d", &k);
            float value = (*Pi)(k);
            printf("Pi(%d) = %f\n", k, value);
        }
        else if (command == 2)
        {
            int* (*Sort)(int*, int) = dlsym(dl_handler, FUNCTION2_NAME);
            check_wrong(Sort, NULL);
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
            array = (*Sort)(array, n);
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
    check_ok(dlclose(dl_handler), 0);
    return 0;
}

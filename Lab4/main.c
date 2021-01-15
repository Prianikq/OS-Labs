#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s\n", MSG); return 1; }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { printf("%s\n", MSG); return 1; }
char* SHARED_FILE_NAME = "pipe";
const int SIZE_SHARED_FILE = 12;
int main()
{
    /* Work with input data file */
    char input_file[256];
    printf("Input file name: ");
    fgets(input_file, 255, stdin);
    if (input_file[strlen(input_file)-1] == '\n') // cause fgets adding in string \n
    {
        input_file[strlen(input_file)-1] = '\0';
    }
    FILE* input = fopen(input_file, "r");
    check_wrong(input, NULL, "Error creating input data file!");

    /* Work with shared file */
    int sf_d = shm_open(SHARED_FILE_NAME, O_RDWR | O_CREAT, S_IRWXU);
    check_wrong(sf_d, -1, "Error creating shared data file!");
    check_ok(ftruncate(sf_d, SIZE_SHARED_FILE), 0, "Error changing size of shared data file!");
    char* data = mmap(NULL, SIZE_SHARED_FILE, PROT_READ | PROT_WRITE, MAP_SHARED, sf_d, 0);
    check_wrong(data, MAP_FAILED, "Error mapping shared data file!");
    for (int i = 0; i < SIZE_SHARED_FILE; ++i)
    {
        data[i] = '\0';
    }
    
    /* Work with signal variable */
    sigset_t set;
    check_ok(sigemptyset(&set), 0, "Error creating signal set");
    check_ok(sigaddset(&set, SIGUSR1), 0, "Error adding signal in set");
    check_ok(sigaddset(&set, SIGUSR2), 0, "Error adding signal in set");
    check_ok(sigprocmask(SIG_BLOCK, &set, NULL), 0, "Error adding blocking signals in main!");
    
    int id = fork();
    check_wrong(id, -1, "Fork creating error!");
    if (id == 0)
    {
        printf("[%d] It's child process\n", getpid());
     	char* arg[] = {"child", SHARED_FILE_NAME, NULL};
        check_wrong(dup2(fileno(input), 0), -1, "Error creating duplicate file descriptor for input file!");
     	check_wrong(execv("child", arg), -1, "Error when starting a child for execution!");
    }        
    else
    {
        printf("[%d] It's parent process of %d\n", getpid(), id);
        int sg;
        check_ok(sigwait(&set, &sg), 0, "Error waiting signal from child process!");
        check_ok(sg, SIGUSR2, "Error! Unknown signal from child process!");
        while (1)
        {
            check_ok(kill(id, SIGUSR1), 0, "Error sending signal to child process!");
            check_ok(sigwait(&set, &sg), 0, "Error waiting signal from child process!");
            check_ok(sigaddset(&set, sg), 0, "Error adding signal in set");
            printf("%s", data);
            for (int i = 0; i < SIZE_SHARED_FILE; ++i)
            {
                data[i] = '\0';
            }
            if (sg == SIGUSR2)
            {
                break;
            }
        }
    }
    check_ok(munmap(data, SIZE_SHARED_FILE), 0, "Error unmapping shared file in main process!");
    check_wrong(shm_unlink(SHARED_FILE_NAME), -1, "Error unlink shared file in main process!");
    check_ok(fclose(input), 0, "Error closing input file!");
}

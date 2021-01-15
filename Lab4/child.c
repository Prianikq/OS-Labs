#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>  
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> 
#define check_ok(VALUE, OKVAL, MSG) if (VALUE != OKVAL) { printf("%s\n", MSG); return 1; }
#define check_wrong(VALUE, WRONGVAL, MSG) if (VALUE == WRONGVAL) { printf("%s\n", MSG); return 1; }

void itoa10(int val, char* data, int index){
    int old_index = index;
    while (val != 0)
    {
        data[index] = val % 10 + '0';
        val /= 10;
        ++index;
    }
    for (int i = old_index; i < old_index+(index-old_index)/2; ++i)
    {
        char c = data[i];
        data[i] = data[index-i+old_index-1];
        data[index-i+old_index-1] = c;
    }
}

int main(int argc, char* argv[])
{
    check_ok(argc, 2, "Wrong arguments in child process!");
    
    /* Open shared file */
    int sf_d = shm_open(argv[1], O_RDWR, S_IRWXU);
    check_wrong(sf_d, -1, "Error open shared data file!");
    struct stat statbuf;
    check_wrong(fstat(sf_d, &statbuf), -1, "Error getting file stat in child!");
    char* data = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, sf_d, 0);
    check_wrong(data, MAP_FAILED, "Error mapping shared data file in child process!");
    
    /* Working with signal set */
    sigset_t set;
    check_ok(sigemptyset(&set), 0, "Error creating signal set");
    check_ok(sigaddset(&set, SIGUSR1), 0, "Error adding signal in set");
    check_ok(sigprocmask(SIG_BLOCK, &set, NULL), 0, "Error adding blocking signals in child!");
    
    check_ok(kill(getppid(), SIGUSR2), 0, "Error sending signal to main process!");
    
    int num;
    int index = 0;
    char buffer[statbuf.st_size];
    while (scanf("%d", &num) > 0)
    {
    	int is_prime = 1;
    	if (num >= 0)
    	{
    	    for (int i = 2; i * i <= num; ++i)
	        {
	            if (num % i == 0)
	            {	
	    	        is_prime = 0;
	    	        break;
	            }
	        }
    	}
        if (is_prime == 0) 
        {
            int copy = num;
	        int size_num = 0;
	        while (copy != 0)
	        {
	            ++size_num;
	            copy /= 10;
	        }
            if (index + size_num + 1 >= statbuf.st_size - 1)
            {
                int sd;
                check_ok(sigwait(&set, &sd), 0, "Error waiting signal!");
                check_ok(sd, SIGUSR1, "Error! Unknown signal from main process!");
                check_ok(sigaddset(&set, SIGUSR1), 0, "Error adding signal in set");
                for (int i = 0; i < index; ++i)
                {
                     data[i] = buffer[i];
                     buffer[i] = '\0';
                }       
                kill(getppid(), SIGUSR1);
                index = 0;
            }
            itoa10(num, buffer, index);
            index += size_num;
            buffer[index] = '\n';
            ++index;
        }
        else
        {
            int sd;
            check_ok(sigwait(&set, &sd), 0, "Error waiting signal!");
            check_ok(sd, SIGUSR1, "Error! Unknown signal from main process!");
            for (int i = 0; i < index; ++i)
            {           
                data[i] = buffer[i];
                buffer[i] = '\0';
            }
            kill(getppid(), SIGUSR2);
            break;
        }
    }
    check_ok(munmap(data, statbuf.st_size), 0, "Error unmapping shared file in child process!");
}

#include "unistd.h"
#include "stdio.h"
int main()
{
     char input[256];
     printf("Enter file name: ");
     scanf("%s", input);
     FILE* fp;
     if ((fp = fopen(input, "r")) == NULL)
     {
	     perror("Error! Input file isn't opened!");
     	 return -1;
     }
     int fd[2];
     if (pipe(fd) == -1)
     {
         perror("Error! Pipe isn't created!");
         return -2;
     }
     int id = fork();
     if (id == -1)
     {
         perror("Fork error!");
         return -3;
     }
     if (id == 0)
     {
     	 printf("[%d] It's child process\n", getpid());
     	 close(fd[0]);
     	 char* arg[] = {"child", NULL};
     	 if (dup2(fd[1], 1) == -1)
         {
             perror("Error creating duplicate file descriptor for pipe output!\n");
             return -4;
         }
         if (dup2(fileno(fp), 0) == -1)
	     {
	         perror("Error creating duplicate file descriptor for input file!\n");
	         return -5;
	     }
     	 if (execv("child", arg) == -1)
     	 {
     	     perror("Error when starting a child for execution!");
     	     return -6;
     	 }
     }
     else
     {
         printf("[%d] It's parent process of %d\n", getpid(), id);
         close(fd[1]);
         int res;
         while (read(fd[0], &res, sizeof(int)) != 0)
	     {
	         printf("%d is composit number\n", res);
	     }
     }
     return 0;
}
        

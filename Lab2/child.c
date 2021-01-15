 #include "stdio.h"
 #include "unistd.h"
 int main()
{
    int num;
    while (scanf("%d", &num) > 0)
    {
    	int is_prime = 1;
    	if (num < 0) break;
    	for (int i = 2; i * i <= num; ++i)
	    {
	        if (num % i == 0)
	        {	
	    	    is_prime = 0;
	    	    break;
	        }
	    }
        if (is_prime == 0) write(1, &num, sizeof(int));
	    else break;
    }
}

#include "functions.h"
float Pi(int k)
{
    float value = 0;
    for (int i = 0; i <= k; ++i)
    {
        value += 1.0 * (i % 2 == 0? 1 : -1) / (2 * i + 1);
    }
    return value * 4;
}
int* Sort(int* array, int n)
{
	int oper = 1;
	while (oper)
	{
		oper = 0;
		for (int i = 1; i < n; ++i)
		{
			if (array[i-1] > array[i])
			{
				int swap = array[i];
				array[i] = array[i-1];
				array[i-1] = swap;
				oper = 1;
			}
		}
	}
	return array;
}

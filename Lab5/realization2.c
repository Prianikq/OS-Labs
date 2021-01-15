#include "functions.h"
float Pi(int k)
{
    float value = 1;
    for (int i = 1; i <= k; ++i)
    {
        value *= 4.0 * i * i / ((2 * i - 1) * (2 * i + 1));
    }
    return value * 2;
}
void FastSort(int* a, int l, int r)
{
	if (l < r)
	{
		int v = a[(l + r) / 2];
		int i = l;
		int j = r;
		while (i <= j)
		{
			while (a[i] < v) ++i;
			while (a[j] > v) --j;
			if (i >= j) break;
			else
			{
				int swap = a[i];
				a[i] = a[j];
				a[j] = swap;
				++i;
				--j;
			}
		}
		FastSort(a, l, j);
		FastSort(a, j + 1, r);
	}
}
int* Sort(int* array, int n)
{
	FastSort(array, 0, n - 1);
	return array;
}


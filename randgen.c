#include "randgen.h"

#define m 100000000
#define m1 10000
#define b 31415821

static int j;

static int a[55];

int mult(int p, int q)
{
	int p1, p0, q1, q0;
	p1 = p/m1; p0 = p % m1;
	q1 = q/m1; q0 = q % m1;

	return (((p0*q1+p1*q0) % m1)*m1+p0*q0) % m;
}

void randinit(int s)
{
	for(a[0] = s, j = 0; j <= 54; j++)
		a[j] = (mult(a[j-1],b)+1) % m;
}

int random(int r)
{
	j = (j+1) % 55;
	a[j] = (a[(j+23) % 55] + a[(j+54) % 55]) % m;
	return ((a[j]/m1)*r)/m1;
}

#include <stdio.h>

static int	w = 1;
static int	z = 2;
	
void init_random(int seed)
{
	w = seed + 1;
	z = seed * seed + seed + 2;
}

int next(void)
{
	int	x;

	z = 36969 * (z & 65535) + (z >> 16);
	w = 18000 * (w & 65535) + (w >> 16);

	x = (z << 16) + w;

	return x;
}

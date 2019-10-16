#include <stdio.h>
#include <string.h>

#define N (2048)

float sum;
float a[N][N];
float b[N][N];
float c[N][N];

void matmul()
{
	int	i, j, k;

	#pragma omp parallel private(i,j,k)
	#pragma omp for schedule(static, N/omp_get_num_procs())
	for (i = 0; i < N; i += 1) {
		for (j = 0; j < N; j += 1) {
			a[i][j] = 0;
			for (k = 0; k < N; k += 1) {
				a[i][j] += b[i][k] * c[k][j];
			}
		}
	}
}

void init()
{
	int	i, j;

	for (i = 0; i < N; i += 1) {
		for (j = 0; j < N; j += 1) {
			b[i][j] = 12 + i * j * 13;
			c[i][j] = -13 + i + j * 21;
		}
	}
}

void check()
{
	int	i, j;

	for (i = 0; i < N; i += 1)
		for (j = 0; j < N; j += 1)
			sum += a[i][j];
	printf("sum = %lf\n", sum);
}

int main()
{
	init();
	matmul();
	check();

	return 0;
}

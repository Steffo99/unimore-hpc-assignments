#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "atax.hu"

// Workaround for the editor not finding M_PI
// It is exclusive to the GNU C compiler
// https://www.gnu.org/software/libc/manual/html_node/Mathematical-Constants.html
#ifndef M_PI
	#define M_PI 3.141
#endif

/**
 * Initialize the arrays to be used in the computation:
 * 
 * - `x` is filled with multiples of `M_PI`;
 * - `A` is filled with sample data.
 * 
 * To be called on the CPU (uses the `__host__` qualifier).
 */
__host__ static void init_array(int nx, int ny, DATA_TYPE POLYBENCH_2D(A, NX, NY, nx, ny), DATA_TYPE POLYBENCH_1D(x, NY, ny))
{
	for (int i = 0; i < ny; i++) {
		x[i] = i * M_PI;
	}

	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < ny; j++) {
			A[i][j] = ((DATA_TYPE)i * (j + 1)) / nx;
		}
	}
}

/** 
 * Print the given array. 
 * 
 * Cannot be parallelized, as the elements of the array should be 
 * 
 * To be called on the CPU (uses the `__host__` qualifier).
 */
__host__ static void print_array(int nx, DATA_TYPE POLYBENCH_1D(y, NX, nx))
{
	int i;

	for (i = 0; i < nx; i++) {
		fprintf(stderr, DATA_PRINTF_MODIFIER, y[i]);
	}
	fprintf(stderr, "\n");
}


/**
 * Compute ATAX.
 * 
 * Parallelizing this is the goal of the assignment.
 * 
 * Currently to be called on the CPU (uses the `__host__` qualifier), but we may probably want to change that soon.
 */
__host__ static void kernel_atax(int nx, int ny, DATA_TYPE POLYBENCH_2D(A, NX, NY, nx, ny), DATA_TYPE POLYBENCH_1D(x, NY, ny), DATA_TYPE POLYBENCH_1D(y, NY, ny))
{
	for (int i = 0; i < _PB_NY; i++) {
		y[i] = 0;
	}
	
	for (int i = 0; i < _PB_NX; i++) {
		DATA_TYPE tmp = 0;
		
		for (int j = 0; j < _PB_NY; j++) {
			tmp += A[i][j] * x[j];
		}
		
		for (int j = 0; j < _PB_NY; j++) {
			y[j] = y[j] + A[i][j] * tmp;
		}
	}
}

/**
 * The main function of the benchmark, which sets up tooling to measure the time spent computing `kernel_atax`.
 * 
 * We should probably avoid editing this.
 */
__host__ int main(int argc, char **argv)
{
	int nx = NX;
	int ny = NY;

	POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, NX, NY, nx, ny);
	POLYBENCH_1D_ARRAY_DECL(x, DATA_TYPE, NY, ny);
	POLYBENCH_1D_ARRAY_DECL(y, DATA_TYPE, NY, ny);

	init_array(nx, ny, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x));
	
	polybench_start_instruments;

	kernel_atax(nx, ny, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x), POLYBENCH_ARRAY(y));

	polybench_stop_instruments;
	polybench_print_instruments;

	/* Prevent dead-code elimination. All live-out data must be printed by the function call in argument. */
	polybench_prevent_dce(print_array(nx, POLYBENCH_ARRAY(y)));
	
	POLYBENCH_FREE_ARRAY(A);
	POLYBENCH_FREE_ARRAY(x);
	POLYBENCH_FREE_ARRAY(y);

	return 0;
}
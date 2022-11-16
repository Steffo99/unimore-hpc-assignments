#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "atax.h"

// Workaround for the editor not finding M_PI
// It is exclusive to the GNU C compiler
// https://www.gnu.org/software/libc/manual/html_node/Mathematical-Constants.html
#ifndef M_PI
#define M_PI 3.141
#endif

/* Array initialization. */
static void init_array(int nx, int ny,
                       DATA_TYPE POLYBENCH_2D(A, NX, NY, nx, ny),
                       DATA_TYPE POLYBENCH_1D(x, NY, ny))
{
  int i, j;

  /// Initialize the `x` array with PI and its multiples.
  #pragma omp parallel for num_threads(4) schedule(static)
  for (i = 0; i < ny; i++) {
    x[i] = i * M_PI;
  }

  /// Initialize the `A` matrix with [something?]
  // Using 4 threads here slows everything down: why?
  // #pragma omp parallel for num_threads(4) schedule(static)
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
      A[i][j] = ((DATA_TYPE)i * (j + 1)) / nx;
}

/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static void print_array(int nx,
                        DATA_TYPE POLYBENCH_1D(y, NX, nx))
{
  int i;

  // Cannot optimize this: prints have to be dependent on each other to make sense!

  for (i = 0; i < nx; i++)
  {
    fprintf(stderr, DATA_PRINTF_MODIFIER, y[i]);
    if (i % 20 == 0)
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static void kernel_atax(int nx, int ny,
                        DATA_TYPE POLYBENCH_2D(A, NX, NY, nx, ny),
                        DATA_TYPE POLYBENCH_1D(x, NY, ny),
                        DATA_TYPE POLYBENCH_1D(y, NY, ny),
                        DATA_TYPE POLYBENCH_1D(tmp, NX, nx))
{
  int i, j;

  // TODO: Optimizable loop, no dependencies
  for (i = 0; i < _PB_NY; i++)
    y[i] = 0;
  
  // TODO: Optimizable loop, no dependencies
  // actually tmp[i] could be a local variable
  for (i = 0; i < _PB_NX; i++)
    tmp[i] = 0;

  // TODO: what do we do here?
  for (i = 0; i < _PB_NX; i++)
  {
    for (j = 0; j < _PB_NY; j++)
      tmp[i] = tmp[i] + A[i][j] * x[j];
    for (j = 0; j < _PB_NY; j++)
      y[j] = y[j] + A[i][j] * tmp[i];
  }
}

int main(int argc, char **argv)
{
  /* Retrieve problem size. */
  int nx = NX;
  int ny = NY;

  /* Variable declaration/allocation. */
  POLYBENCH_2D_ARRAY_DECL(A, DATA_TYPE, NX, NY, nx, ny);
  POLYBENCH_1D_ARRAY_DECL(x, DATA_TYPE, NY, ny);
  POLYBENCH_1D_ARRAY_DECL(y, DATA_TYPE, NY, ny);
  POLYBENCH_1D_ARRAY_DECL(tmp, DATA_TYPE, NX, nx);

  /* Initialize array(s). */
  init_array(nx, ny, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_atax(nx, ny,
              POLYBENCH_ARRAY(A),
              POLYBENCH_ARRAY(x),
              POLYBENCH_ARRAY(y),
              POLYBENCH_ARRAY(tmp));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(nx, POLYBENCH_ARRAY(y)));
  
  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x);
  POLYBENCH_FREE_ARRAY(y);
  POLYBENCH_FREE_ARRAY(tmp);

  return 0;
}

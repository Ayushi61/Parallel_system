#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "jemalloc/jemalloc.h"

#define BACK_FILE "/tmp/arajend4.app.back" //COMMENT: remove this comment after
#define MMAP_FILE "/tmp/arajend4.app.mmap"// COMMENT: entering you unity-id
#define MMAP_SIZE ((size_t)1 << 30)

#define _USE_MATH_DEFINES

#define XMIN 0.0
#define XMAX 1.0
#define YMIN 0.0
#define YMAX 1.0

#define MAX_PSZ 10
#define TSCALE 1.0
#define VSQR 0.1

void init(double *u, double *pebbles, int n);
void evolve(double *un, double *uc, double *uo, double *pebbles, int n, double h, double dt, double t);
int tpdt(double *t, double dt, double end_time);
void print_heatmap(const char *filename, double *u, int n, double h);
void init_pebbles(double *p, int pn, int n);

void run_cpu(double *u, double *u0, double *u1, double *pebbles, int n, double h, double end_time);

PERM double t;
PERM double elapsed_cpu;
PERM double *pebs, *u_i0, *u_i1;
PERM struct timeval cpu_start, cpu_end;
//extern void run_gpu(double *u, double *u0, double *u1, double *pebbles, int n, double h, double end_time, int nthreads);

int main(int argc, char *argv[])
{

 /* if(argc != 5)
  {
    printf("Usage: %s npoints npebs time_finish nthreads \n",argv[0]);
    return 0;
  }
*/
   int do_restore = argc > 1 && strcmp("-r", argv[1]) == 0;
  const char *mode = (do_restore) ? "r+" : "w+";
  
  // Persistent memory initialization
     perm(PERM_START, PERM_SIZE);
       mopen(MMAP_FILE, mode, MMAP_SIZE);
        bopen(BACK_FILE, mode);
 
        // Init persistent variables
  int     npoints   = 128;
  int     npebs     = 8;
  double  end_time  = 1.0;
  int     nthreads  = atoi(argv[4]);
  int 	  narea	    = npoints * npoints;
//  double *u_i0, *u_i1;
  double *u_cpu;// *u_gpu, *pebs;
  double h;

//  double elapsed_cpu, elapsed_gpu;
//  struct timeval cpu_start, cpu_end, gpu_start, gpu_end;
  
  u_i0 = (double*)malloc(sizeof(double) * narea);
  u_i1 = (double*)malloc(sizeof(double) * narea);
  pebs = (double*)malloc(sizeof(double) * narea);

  	h = (XMAX - XMIN)/npoints;
  u_cpu = (double*)malloc(sizeof(double) * narea);
  //u_gpu = (double*)malloc(sizeof(double) * narea);

  if (!do_restore) {
	t = 0.0;
	elapsed_cpu=0.0;
  	printf("Running %s with (%d x %d) grid, until %f, with %d threads\n", argv[0], npoints, npoints, end_time, nthreads);


  	init_pebbles(pebs, npebs, npoints);
  	init(u_i0, pebs, npoints);
  	init(u_i1, pebs, npoints);

  	
  	print_heatmap("lake_i.dat", u_i0, npoints, h);
	mflush();
	backup();
	}
	else
	{
		printf("restarting...\n");
		restore();
		// Calculating elapsed times in previous runs.
		elapsed_cpu += ((cpu_end.tv_sec + cpu_end.tv_usec * 1e-6)-(cpu_start.tv_sec + cpu_start.tv_usec * 1e-6));
	}
  	gettimeofday(&cpu_start, NULL);
  run_cpu(u_cpu, u_i0, u_i1, pebs, npoints, h, end_time);
  gettimeofday(&cpu_end, NULL);

  elapsed_cpu = ((cpu_end.tv_sec + cpu_end.tv_usec * 1e-6)-(
                  cpu_start.tv_sec + cpu_start.tv_usec * 1e-6));
  printf("CPU took %f seconds\n", elapsed_cpu);

 /* gettimeofday(&gpu_start, NULL);
  run_gpu(u_gpu, u_i0, u_i1, pebs, npoints, h, end_time, nthreads);
  gettimeofday(&gpu_end, NULL);
  elapsed_gpu = ((gpu_end.tv_sec + gpu_end.tv_usec * 1e-6)-(
                  gpu_start.tv_sec + gpu_start.tv_usec * 1e-6));
  printf("GPU took %f seconds\n", elapsed_gpu);

*/
  print_heatmap("lake_f.dat", u_cpu, npoints, h);

  free(u_i0);
  free(u_i1);
  free(pebs);
  free(u_cpu);
//  free(u_gpu);
  mclose();
 bclose();
remove(BACK_FILE);
remove(MMAP_FILE);
  return 1;
}

void run_cpu(double *u, double *u0, double *u1, double *pebbles, int n, double h, double end_time)
{
  double dt = h / 2.;

  while(1)
  {
    evolve(u, u0, u1, pebbles, n, h, dt, t);

    memcpy(u0, u1, sizeof(double) * n * n);
    memcpy(u1, u, sizeof(double) * n * n);

    if(!tpdt(&t,dt,end_time)) break;
	gettimeofday(&cpu_end, NULL);
	backup();
  }
  
}

void init_pebbles(double *p, int pn, int n)
{
  int i, j, k, idx;
  int sz;

  srand( 0 );
  memset(p, 0, sizeof(double) * n * n);

  for( k = 0; k < pn ; k++ )
  {
    i = rand() % (n - 4) + 2;
    j = rand() % (n - 4) + 2;
    sz = rand() % MAX_PSZ;
    idx = j + i * n;
    p[idx] = (double) sz;
  }
}

double f(double p, double t)
{
  return -expf(-TSCALE * t) * p;
}

int tpdt(double *t, double dt, double tf)
{
  if((*t) + dt > tf) return 0;
  (*t) = (*t) + dt;
  return 1;
}

void init(double *u, double *pebbles, int n)
{
  int i, j, idx;

  for(i = 0; i < n ; i++)
  {
    for(j = 0; j < n ; j++)
    {
      idx = j + i * n;
      u[idx] = f(pebbles[idx], 0.0);
    }
  }
}

void evolve(double *un, double *uc, double *uo, double *pebbles, int n, double h, double dt, double t)
{
  int i, j, idx;

  for( i = 0; i < n; i++)
  {
    for( j = 0; j < n; j++)
    {
      idx = j + i * n;

      if( i == 0 || i == n - 1 || j == 0 || j == n - 1)
      {
        un[idx] = 0.;
      }
      else
      {
        un[idx] = 2*uc[idx] - uo[idx] + VSQR *(dt * dt) *((uc[idx-1] + uc[idx+1] + 
                    uc[idx + n] + uc[idx - n] - 4 * uc[idx])/(h * h) + f(pebbles[idx],t));
      }
    }
  }
}

void print_heatmap(const char *filename, double *u, int n, double h)
{
  int i, j, idx;

  FILE *fp = fopen(filename, "w");  

  for( i = 0; i < n; i++ )
  {
    for( j = 0; j < n; j++ )
    {
      idx = j + i * n;
      fprintf(fp, "%f %f %f\n", i*h, j*h, u[idx]);
    }
  }
  
  fclose(fp);
} 


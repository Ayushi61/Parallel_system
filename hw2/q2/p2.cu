/*


Single Author info:
arajend4 Ayushi Rajendra Kumar

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <cuda_runtime.h>
/* first grid point */
#define   XI              0.0
/* last grid point */
#define   XF              M_PI
typedef   double   FP_PREC;
/* function declarations */
double     fn(double);
void        print_function_data(int, double*, double*, double*);
int         main(int, char**); 
__global__ void calc_area(double*,double*,double*,double,int);
__device__ void fn1(double*, double*);
int main (int argc, char *argv[])
{
        int NGRID;
        if(argc > 1)
            NGRID = atoi(argv[1]);
        else 
        {
                printf("Please specify the number of grid points.\n");
                exit(0);
        }
        //loop index
        int         i;
        double  h, area;

        double *inf = (double *)malloc(sizeof(double) * (NGRID + 1) );
        double  *xc = (double *)malloc(sizeof(double)* (NGRID + 1));
        double  *yc = (double*)malloc(sizeof(double) * (NGRID + 1));
        double *inf_d;
	double *xc_d;
	double *yc_d;
	cudaMalloc((void ** )&inf_d,sizeof(double)*(NGRID+1)); 
	cudaMalloc((void ** )&xc_d,sizeof(double)*(NGRID+1)); 
	cudaMalloc((void ** )&yc_d,sizeof(double)*(NGRID+1)); 
	int blockSize=4;
	int nBlocks = (NGRID)/blockSize + ((NGRID)%blockSize == 0?0:1);
        //construct grid
	for (i = 1; i <= NGRID ; i++)
        {
                xc[i] = XI + (XF - XI) * (double)(i - 1)/(double)(NGRID - 1);
        }
        
        int  imin, imax;  

        imin = 1;
        imax = NGRID;
        //define the function
      //  for( i = imin; i <= imax; i++ )
       // {
         //       yc[i] = fn(xc[i]);
       // }
	


        inf[0] = 0.0;
        h = (XF - XI) / (NGRID - 1);
        
	cudaMemcpy(xc_d,&xc[1],sizeof(double)*(NGRID+1),cudaMemcpyHostToDevice);
	cudaMemcpy(inf_d,inf,sizeof(double)*(NGRID+1),cudaMemcpyHostToDevice);
        calc_area<<<nBlocks,blockSize>>>(inf_d,xc_d,yc_d,h,NGRID);
	cudaMemcpy(&inf[1],inf_d,sizeof(double)*(NGRID),cudaMemcpyDeviceToHost);
	cudaMemcpy(&yc[1],yc_d,sizeof(double)*(NGRID),cudaMemcpyDeviceToHost);
        for(i = 1 ; i <= NGRID; ++i){
           //  x += h;
           //  y2 = fn(x);
         //   area = ( yc[i] + yc[i-1]) * h / 2;
            inf[i] = inf[i]+inf[i-1];
        }


        print_function_data(NGRID, &xc[1], &yc[1], &inf[1]);

	cudaFree(inf_d);
	cudaFree(xc_d);
	cudaFree(yc_d);
        //free allocated memory 
        free(xc);
        free(yc);
        free(inf);

        return 0;
}

//prints out the function and its derivative to a file
void print_function_data(int np, double *x, double *y, double *dydx)
{
        int   i;

        char filename[1024];
        sprintf(filename, "fn-%d.dat", np);

        FILE *fp = fopen(filename, "w");

        for(i = 0; i < np; i++)
        {
                fprintf(fp, "%f %f %f\n", x[i], y[i], dydx[i]);
        }

        fclose(fp);
}


__global__ void calc_area(double *area,double *xc,double *yc,double h,int N)
{
	int idx=blockIdx.x*blockDim.x + threadIdx.x;
	//double y[2];
	

	fn1(&xc[idx],&yc[idx]);
	//fn1(&xc[idx-1],&y[1]);
	//printf("value of yc is idx %d %lf\n",idx,yc[idx]);
	__syncthreads();

	if(idx>0) area[idx]=(yc[idx]+yc[idx-1])*h/2;
//	printf("yc is %lf,h is %d,area is %lf\n",yc[idx],h,area[idx]);
}

__device__ void fn1(double* x,double* y)
{
  *y= *x * *x;
 //  printf("fn is %lf\n",*y);
}



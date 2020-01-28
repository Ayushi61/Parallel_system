#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* first grid point */
#define   XI              0.0
/* last grid point */
#define   XF              M_PI

/* function declarations */
double     fn(double);
void        print_function_data(int, double*, double*, double*);
int         main(int, char**);

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
        
        //construct grid
        for (i = 1; i <= NGRID ; i++)
        {
                xc[i] = XI + (XF - XI) * (double)(i - 1)/(double)(NGRID - 1);
        }
        
        int  imin, imax;  

        imin = 1;
        imax = NGRID;
        //define the function
        for( i = imin; i <= imax; i++ )
        {
                yc[i] = fn(xc[i]);
        }


        inf[0] = 0.0;
        h = (XF - XI) / (NGRID - 1);
        
        for(i = 1 ; i <= NGRID; ++i){
            // x += h;
            // y2 = fn(x);
            area = ( yc[i] + yc[i-1]) * h / 2;
            inf[i] = inf[i-1] + area;
        }


        print_function_data(NGRID, &xc[1], &yc[1], &inf[1]);


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

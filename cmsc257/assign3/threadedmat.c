#include <sys/times.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "base_matrix_functions.c"

#define MIN(a,b) (a > b ? b : a)

double start, stop, used, mf;

double ftime(void);
void multiply (double **a, double **b, double **c, int n);

double ftime (void)
{
        struct tms t;

        times ( &t );

        return (t.tms_utime + t.tms_stime) / 100.0;
}

void multiply (double **a, double **b, double **c, int n)
{
        int i, j, k, i0, j0, k0, pid;

        for (i=0; i<n; i++)
        {
                for (j=0; j<n; j++)

                        c[i][j] = 0;
        }

        int step = 256;
        int forks = 0;

        for (i0 = 0; i0 < n; i0 += step) {
                for (j0 = 0; j0 < n; j0 += step) {
                        for (k0 = 0; k0 < n; k0 += step) {
                                forks++;
                                pid = fork();
                                if ( pid < 0 ) {
                                        fprintf(stderr,"fork failed at %d\n",i);
                                        exit(1);
                                } else if ( pid > 0 ) {
                                        // printf("parent: new child is %d\n",pid);
                                } else {
                                        for (i=i0; i < MIN(i0+step,n); i++) {
                                                for (j=j0; j < MIN(j0+step,n); j++) {
                                                        for (k=k0; k < MIN(k0+step,n); k++) {
                                                                c[i][j]= c[i][j] + a[i][k] * b[k][j];
                                                        }
                                                }
                                        }
                                        _exit(0);
                                }
                        }
                }
        }

        for ( i = 0; i < forks; i++ ) wait(NULL);
}

int main (void)
{
        time_t mytime;
        mytime = time(NULL);
        printf("Report began: %s\n\n", ctime(&mytime));

        // validate matrix multiplication
        struct validationMatrix * validator = buildValidationMatrix(0);

        multiply(validator->a,validator->b,validator->c, 4);

        validate(validator);

        int i, j, n;
        double **a, **b, **c;

        printf ( "Enter the value of n: ");
        scanf ( "%d", &n);
        printf("%d", n);

        //Populate arrays....
        a= (double**)malloc(n*sizeof(double));
        b= (double**)malloc(n*sizeof(double));
        c= (double**)malloc(n*sizeof(double));

        for (i=0; i<n; i++)
        {
                a[i]= malloc(sizeof(double)*n);
                b[i]= malloc(sizeof(double)*n);
                c[i]= malloc(sizeof(double)*n);
        }

        for (i=0; i<n; i++)
        {
                for (j=0; j<n; j++)
                        a[i][j]=8;
        }

        for (i=0; i<n; i++)
        {
                for (j=0; j<n; j++)
                        b[i][j]=7;
        }

        start = ftime();
        multiply(a,b,c,n);
        stop = ftime();
        used = stop - start;
        mf = (n*n*n *2.0) / used / 1000000.0;
        printf ("\n");
        printf ( "Elapsed time:   %10.2f \n", used);
        printf ( "DP MFLOPS:       %10.2f \n", mf);

        mytime = time(NULL);
        printf("\n\nReport completed: %s\n", ctime(&mytime));
        return (0);
}

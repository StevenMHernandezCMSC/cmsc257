#include <sys/times.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

double start, stop, used, mf;

/**
 * we want to validate that a x b = expected, which we will store in c
 */
struct validationMatrix {
        double **a;
        double **b;
        double **c;
        double **expected;
        int size;
};

void arr_copy(double *out, double in[], int n) {
        int i;
        for (i = 0; i < n; i++) {
                out[i] = in[i];
        }
}

struct validationMatrix *buildSharedMemoryMatrix(double *mem) {
        int shmfd;
        /*
         * create shared memory for validation memory
         */
        shmfd = shm_open ( "/hernandez_shr", O_RDWR | O_CREAT, 0666 );
        if ( shmfd < 0 ) {
                fprintf(stderr,"Could not create brs_memory\n");
                exit(1);
        }
        ftruncate (shmfd, 4*4*4*sizeof(double));
        mem = (double *)mmap ( NULL, 4*4*4*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 );
        if ( mem == NULL ) {
                fprintf(stderr,"Could not map brs_memory\n");
                exit(1);
        }
        close ( shmfd );
        shm_unlink ( "/hernandez_shr" );

        struct validationMatrix *myValidationMatrix = malloc(sizeof(struct validationMatrix));

        myValidationMatrix->a = (double**)malloc(4*sizeof(double));
        myValidationMatrix->b = (double**)malloc(4*sizeof(double));
        myValidationMatrix->c = (double**)malloc(4*sizeof(double));
        myValidationMatrix->expected = (double**)malloc(4*sizeof(double));

        int i;

        for (i=0; i<4; i++) {
                myValidationMatrix->a[i] = &mem[i*4];
                myValidationMatrix->b[i] = &mem[(i*4) + 16];
                myValidationMatrix->c[i] = &mem[(i*4) + 32];
                myValidationMatrix->expected[i] = &mem[(i*4) + 48];
        }

        return myValidationMatrix;
}

struct validationMatrix *buildMallocedMatrix() {
        struct validationMatrix *myValidationMatrix = malloc(sizeof(struct validationMatrix));

        myValidationMatrix->a = (double**)malloc(4*sizeof(double));
        myValidationMatrix->b = (double**)malloc(4*sizeof(double));
        myValidationMatrix->c = (double**)malloc(4*sizeof(double));
        myValidationMatrix->expected = (double**)malloc(4*sizeof(double));

        int i;

        for (i=0; i<4; i++) {
                myValidationMatrix->a[i]= malloc(sizeof(double)*4);
                myValidationMatrix->b[i]= malloc(sizeof(double)*4);
                myValidationMatrix->c[i]= malloc(sizeof(double)*4);
                myValidationMatrix->expected[i]= malloc(sizeof(double)*4);
        }
        return myValidationMatrix;
}

void addValidationData(struct validationMatrix *myValidationMatrix, int transpose) {
    double a_1[4] = {9,0,0,2};
    double a_2[4] = {0,3,2,0};
    double a_3[4] = {0,0,0,1};
    double a_4[4] = {0,1,0,0};
    arr_copy(myValidationMatrix->a[0], a_1, 4);
    arr_copy(myValidationMatrix->a[1], a_2, 4);
    arr_copy(myValidationMatrix->a[2], a_3, 4);
    arr_copy(myValidationMatrix->a[3], a_4, 4);

    if (!transpose) {
            double b_1[4] = {4,0,0,9};
            double b_2[4] = {0,0,2,0};
            double b_3[4] = {0,0,0,0};
            double b_4[4] = {0,3,0,0};
            arr_copy(myValidationMatrix->b[0], b_1, 4);
            arr_copy(myValidationMatrix->b[1], b_2, 4);
            arr_copy(myValidationMatrix->b[2], b_3, 4);
            arr_copy(myValidationMatrix->b[3], b_4, 4);
    } else {
            double b_1[4] = {4,0,0,0};
            double b_2[4] = {0,0,0,3};
            double b_3[4] = {0,2,0,0};
            double b_4[4] = {9,0,0,0};
            arr_copy(myValidationMatrix->b[0], b_1, 4);
            arr_copy(myValidationMatrix->b[1], b_2, 4);
            arr_copy(myValidationMatrix->b[2], b_3, 4);
            arr_copy(myValidationMatrix->b[3], b_4, 4);
    }

    double expected_1[4] = {36,6,0,81};
    double expected_2[4] = {0,0,6,0};
    double expected_3[4] = {0,3,0,0};
    double expected_4[4] = {0,0,2,0};
    arr_copy(myValidationMatrix->expected[0], expected_1, 4);
    arr_copy(myValidationMatrix->expected[1], expected_2, 4);
    arr_copy(myValidationMatrix->expected[2], expected_3, 4);
    arr_copy(myValidationMatrix->expected[3], expected_4, 4);
}

struct validationMatrix *buildValidationMatrix(int transpose) {
        struct validationMatrix *myValidationMatrix = buildMallocedMatrix();

        addValidationData(myValidationMatrix, transpose);

        return myValidationMatrix;
}

void validate(struct validationMatrix *validationMatrix) {
        int i, j, m, errors = 0;

        printf("Validating matrix multiplication on smaller matrix\n");
        printf("      A      x      B        =    computed        expected\n");

        for (i = 0; i < 4; i++) { // for each row
                for (m = 0; m < 4; m++) { // for both matrixes (a, b, c and expected)
                        for (j = 0; j < 4; j++) {
                                if (m == 0) {
                                        printf("%2d ", (int)validationMatrix->a[i][j]);
                                } else if (m == 1) {
                                        printf("%2d ", (int)validationMatrix->b[i][j]);
                                } else if (m == 2) {
                                        printf("%2d ", (int)validationMatrix->c[i][j]);
                                } else if (m == 3) {
                                        printf("%2d ", (int)validationMatrix->expected[i][j]);
                                }

                                if (validationMatrix->c[i][j] != validationMatrix->expected[i][j]) {
                                        errors++;
                                }
                        }
                        printf("    ");
                }
                printf("\n");
        }

        printf("%d errors found when validating\n\n", errors / 4);
}

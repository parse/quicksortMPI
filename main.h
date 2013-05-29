
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


void swap(double*, double*);
int partition(double*, int, int, int);
void quickSort(double*, int, int);
double *quickSort_p(MPI_Comm comm, double *local_array, int local_size, int *arrayLength);

void divide(int*, int*, double, double*, int);
int validateSort(double *a, int size);

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include "main.h"
 
// Main program
int main(int argc, char **argv) {
	int *data, *result, *other;
	int m, seed, rank, size, slices, i, step, elements, rest;
	double startTime, endTime, finish;
	MPI_Status status;

	if (argc != 2) {
		printf("Missing arguments");
		return 0; 
	}

	seed = time(NULL);
  srand(seed);
  elements = atoi(argv[1]); 

	MPI_Init(&argc, &argv);								// Initialize MPI
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);	// Get my nymber
	MPI_Comm_size(MPI_COMM_WORLD,&size);	// Get size

	// Check if we are the master process
	if (rank == 0) {	
		srandom(clock());
		
		slices = elements / size;
		rest = elements % size;

		// Fill our buffer with test data
		data = (int *)malloc((elements+2*slices-rest)*sizeof(int));
		for (i = 0; i <= elements; i++) {
			data[i] = (random() % 999 +1);
		}

		if (DEBUG) {
	    printf("\n\nUnsorted array is:  ");
	    for(i = 0; i < elements; ++i) {
	      printf(" %d ", data[i]);
	    }
	    printf("\n");
	  }

	  // Make sure the elements fit in the number of slices calculated
		if (rest != 0) {
			slices++;
		}

		// Start timer
		startTime = MPI_Wtime();

		// Storage for our finalized sorted array
		result = (int *)malloc(slices*sizeof(int));

		// Broadcast and scatter data
		MPI_Bcast(&slices, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Scatter(data, slices, MPI_INT, result, slices, MPI_INT, 
			0, MPI_COMM_WORLD);

		quicksort(result, 0, slices-1);
	} else {
		// Not the master process
		MPI_Bcast(&slices, 1, MPI_INT, 0, MPI_COMM_WORLD);

		result = (int *)malloc(slices*sizeof(int));
		MPI_Scatter(data, slices, MPI_INT, result, slices, MPI_INT, 
			0, MPI_COMM_WORLD);
		quicksort(result, 0, slices-1);
	}

	step = 1;
	while (step < size) {
		// Check if current rank is a parent process
		if (rank % (2 * step) == 0) {
			if (rank+step < size) {
				// Store results and merge them
				MPI_Recv(&m, 1, MPI_INT, rank+step, 0, MPI_COMM_WORLD, &status);
				other = (int *)malloc(m*sizeof(int));
				MPI_Recv(other, m, MPI_INT, rank+step, 0, MPI_COMM_WORLD, &status);
				result = merge(result, slices, other, m);

				// Add resulting slices
				slices = slices+m;
			} 
		} else {
			MPI_Send(&slices, 1, MPI_INT, rank-step, 0, MPI_COMM_WORLD);
			MPI_Send(result, slices, MPI_INT, rank-step, 0, MPI_COMM_WORLD);

			break;
		}

		step = step*2;
	}

	if (rank == 0) {
		if (DEBUG) {
	    printf("Sorted array is:  ");
	    for(i = 1; i < slices; ++i) {
	      printf(" %d ", result[i]);
	    }
	    printf("\n\n");
	  }

	  // End timing and output results
		endTime = MPI_Wtime();
		finish = endTime-startTime;
		printf("Execution time: %.10f\n", finish);		
	}

	MPI_Finalize();

	return 0;
}

// Perform quicksort
void quicksort(int *v, int left, int right) {
	// Check if we should continue or not
	if (left >= right) {
		return;
	}

	int i,last;

	// Swap elements
	swap(v, left, (left+right)/2);
	last = left;

	for (i=left+1;i<=right;i++) {
		if (v[i] < v[left]) {
			// Swap elements
			swap(v, ++last, i);
		}
	}

	// Swap elements
	swap(v, left, last);

	// Do recursive step
	quicksort(v, left, last-1);
	quicksort(v, last+1, right);
}

// Inline swap of two numbers
void swap(int *v, int i, int j) {
	int t;
	t = v[i];
	v[i] = v[j];
	v[j] = t;
}

// Merge two arrays 
int *merge(int a[], int m, int b[], int n) {
  int i, j = 0, k = 0;
  int *result;
  result = (int *)malloc((m+n)*sizeof(int));
  
  for (i = 0; i < m + n;) {
    if (j < m && k < n) {
      if (a[j] < b[k]) {
        result[i] = a[j];
        j++;
      }
      else {
        result[i] = b[k];
        k++;
      }

      i++;
    }
    else if (j == m) {
      for (; i < m + n;) {
        result[i] = b[k];
        k++; i++;
      }
    }
    else {
      for (; i < m + n;) {
        result[i] = a[j];
        j++; i++;
      }
    }
  }

  return result;
}

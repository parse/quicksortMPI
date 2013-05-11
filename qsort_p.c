#include "qsort_p.h"

// Main function
int main (int argc, char *argv[]) {
  int rank, nproc, size, i, level = 0, chunk, length;
  double *data, *local_array, *local_left, *local_right, startTime, endTime, finish, *receiveArray;

  // Init MPI
  MPI_Init(&argc, &argv);                
  MPI_Comm_size(MPI_COMM_WORLD, &nproc); 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

  if (argc != 2) {
    printf("Missing arguments, usage:\n mpirun -np <number of processors> qsort <number of elements>\n");
    return 0; 
  }
  
  // Set global problem size
  size = atoi(argv[1]);
  chunk = size/nproc;
  local_array = (double*)calloc(chunk, sizeof(double));

  // Main processor, fill up array
  if (rank == 0) {
    data = (double *)calloc(size, sizeof(double));

    for (i = 0; i < size; i++) {
      data[i] = (random() % 9999 +1);
    }

    // Start timer
    startTime = MPI_Wtime();
  }

  MPI_Scatter(data, chunk, MPI_DOUBLE, local_array, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);  

  // Quicksort serially
  quickSort(local_array, 0, chunk-1);

  // Quicksort in parallel
  receiveArray = quickSort_p(0, MPI_COMM_WORLD, local_array, chunk, &length);

  int *placement = (int*)calloc(nproc, sizeof(int));
  int *processor_size = (int*)calloc(nproc, sizeof(int));

  // Send the sizes of each processor to processor 0
  MPI_Gather(&length, 1, MPI_INT, processor_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Move values around so we get high and low values merged
  placement[0] = 0;
  for (i = 1; i < nproc; i++) {
    placement[i] = placement[i-1] + processor_size[i-1];
  }

  // Gather a list of sizes into our global array
  MPI_Gatherv(receiveArray, length, MPI_DOUBLE, data, processor_size, placement, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    endTime = MPI_Wtime();
    finish = endTime-startTime;

    // Print runtime info
    printf("%.10f\n", finish);  
    //printf( validateSort(data, size) ? "Successfully sorted array\n" : "Failed sorting array\n");

    free(data);
  }

  MPI_Finalize(); 
  return 0;
}

// Perform serial quicksort
void quickSort(double *arr, int left, int right) {
 if (left < right){
    int pivotIndex = (right+left)/2;
    int pivotNewIndex = partition(arr, left, right, pivotIndex);

    quickSort(arr, left, pivotNewIndex - 1);
    quickSort(arr, pivotNewIndex + 1, right);
  }
}

// Quicksort in parallel
double *quickSort_p (int level, MPI_Comm comm, double *local_array, int local_size, int *arrayLength) {
  int i, local_rank, other_size, local_nproc, exchange_pid;
  double pivot, *received_array, *new_local;

  MPI_Request request;
  MPI_Status status;

  // extract local rank and local nproc
  MPI_Comm_rank(comm, &local_rank);
  MPI_Comm_size(comm, &local_nproc);
  
  if (local_nproc > 1) {
    // Choose pivot
    if (local_rank == 0) {
      pivot = local_array[local_size/2];
    }

    // Broadcast pivot
    MPI_Bcast(&pivot, 1, MPI_DOUBLE, 0, comm);
    
    int lt_pivot = 0, gt_pivot = 0;
    countPartitions(&lt_pivot, &gt_pivot, pivot, local_array, local_size);

    // Divide the processors into two sets and exchange data pairwise between processors in the 
    // two sets such that the processors in one set gets data smaller than the pivot
    if (local_rank < local_nproc/2) {
      exchange_pid = local_rank+(local_nproc/2);
      // Send values to the right of our pivot element to higher ranked processors
      MPI_Isend(local_array+lt_pivot, gt_pivot, MPI_DOUBLE, exchange_pid, 111, comm, &request);

      // Receive data
      // Block until we find the pid on the communicator
      MPI_Probe(exchange_pid, 222, comm, &status);

      // Fetch number of elements
      MPI_Get_count(&status, MPI_DOUBLE, &other_size);

      received_array = (double *)calloc(other_size,sizeof(double));

      // Receive all lower values than the pivot element that is on the right size
      MPI_Recv(received_array, other_size, MPI_DOUBLE, exchange_pid, 222, comm, &status);
      MPI_Wait(&request, &status);

      // Recalculate the new size for the merged array
      local_size = lt_pivot + other_size;
      new_local = (double *) calloc(local_size, sizeof(double));

      // Merge
      int m = 0, n = 0, i;
      for(i = 0; i < local_size; ++i) {
        if(m < lt_pivot && n < other_size) {
          if(local_array[m] <= received_array[n]) {
            new_local[i] = local_array[m++];
          } else {
            new_local[i] = received_array[n++];
          }
        } else if(m < lt_pivot) {
          new_local[i] = local_array[m++];
        } else if(n < other_size) {
          new_local[i] = received_array[n++];
        }
      }
    }

    // Divide the processors into two sets and exchange data pairwise between processors in the 
    // two sets such that the processors in one set gets data larger than the pivot
    if (local_rank >= local_nproc/2) {
      // Send values to the left of our pivot element to lower ranked processors
      exchange_pid = local_rank-(local_nproc/2);
      MPI_Isend(local_array, lt_pivot, MPI_DOUBLE, exchange_pid, 222, comm, &request);

      // Receive data
      // Block until we find the value 'rank+size/2' on the communicator
      MPI_Probe(exchange_pid, 111, comm, &status);
      MPI_Get_count(&status, MPI_DOUBLE, &other_size);

      // Receive all higher values than the pivot element that is on the left size
      received_array = (double *)calloc(other_size,sizeof(double));  
      MPI_Recv(received_array, other_size, MPI_DOUBLE, exchange_pid, 111, comm, &status);
      MPI_Wait(&request, &status);

      // Recalculate the new size for the merged array 
      local_size = other_size + gt_pivot;
      new_local = (double *) calloc(local_size, sizeof(double));

      // Merge
      int m = 0, n = 0, i;
      double *right = local_array+lt_pivot;
      for(i = 0; i < other_size+gt_pivot; ++i) {
        if (m < other_size && n < gt_pivot) {
          if(received_array[m] <= right[n]) {
            new_local[i] = received_array[m++];
          } else {
            new_local[i] = right[n++];
          }
        } else if(m < other_size) {
          new_local[i] = received_array[m++];
        } else if(n < gt_pivot) {
          new_local[i] = right[n++];
        }
      }
    }

    free(local_array);
    free(received_array);

    // Create new communicators for the recursive calls
    MPI_Comm new_comm;

    // Separate by how many different subgroups we need to create
    int split = local_rank / (local_nproc/2);

    // Split the communicator into sub communicators
    MPI_Comm_split(comm, split, 0, &new_comm);

    *arrayLength = local_size;
    return quickSort_p(level+1, new_comm, new_local, local_size, arrayLength);
  } else {
    *arrayLength = local_size;
    return local_array;
  }
}

// Partition a partition according to the pivotindex
int partition (double *arr, int left, int right, int pivotIndex){
  double pivotValue = arr[pivotIndex], temp = arr[right];
  int storeIndex = left, i;

  arr[right] = arr[pivotIndex];
  arr[pivotIndex] = temp;

  for(i = left; i < right; i++) {
    if (arr[i] < pivotValue) {
      temp = arr[storeIndex];
      arr[storeIndex] = arr[i];
      arr[i] = temp;
      storeIndex++;
    }
  }

  temp = arr[right];
  arr[right] = arr[storeIndex];
  arr[storeIndex] = temp;

  return storeIndex;
}

// Swap two values
void swap (double *x, double *y) {
  double temp = *x;
  *x = *y;
  *y = temp;
}

// Check if array is sorted
int validateSort (double *a, int size) {
  for (int i = 0; i < size-1; i++) {
    if (a[i] > a[i+1]) {
      printf("%f - %f\n", a[i],a[i+1]);
      return 0;
    }
  }

  return 1;
}

void countPartitions (int *lt_pivot, int *gt_pivot, double median, double *local_array, int local_size) {
  int i;

  if (local_size < 4) {
    for (i = 0; i < local_size; ++i) {
      if (local_array[i] <= median) {
        ++(*lt_pivot);
      } else {
        ++(*gt_pivot);
      }
    }
  } else {
    int el = local_size / 2;

    if (local_array[el] <= median) {
      for (i = el; i < local_size; ++i) {
        if (local_array[i] > median) {
          break;
        }
      }

      *lt_pivot = i; *gt_pivot = local_size-i;
    } else if (local_array[el] > median) {
      for(i = el; i >= 0; --i) {
        if(local_array[i] <= median) {
          break;
        }
      }

      *lt_pivot = i+1; 
      *gt_pivot = local_size-i-1;
    }
  }
}

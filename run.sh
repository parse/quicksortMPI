#!/bin/bash

file='result.output'

> file

echo "Quicksort started"
for n in 1000000 2000000 3000000 4000000 5000000
do
	echo "n = $n started"
	echo "n = $n" >> $file
	for t in 1 2 4 8 16 32 64
	do
		echo "t = $t" >> $file
        mpirun -hostfile nodes -mca plm_rsh_agent rsh -np $t qsort $n >> $file
        mpirun -hostfile nodes -mca plm_rsh_agent rsh -np $t qsort $n >> $file
        mpirun -hostfile nodes -mca plm_rsh_agent rsh -np $t qsort $n >> $file
        mpirun -hostfile nodes -mca plm_rsh_agent rsh -np $t qsort $n >> $file
		echo "t = $t completed"
	done
done
echo "fox completed"

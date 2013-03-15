#!/bin/bash

file='result.output'

> file

echo "Quicksort started"
for n in 1000 10000 100000 1000000 10000000 100000000
do
	echo "n = $n started"
	echo "n = $n" >> $file
	for t in 1 4 8 16 32
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

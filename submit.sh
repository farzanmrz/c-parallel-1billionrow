#!/bin/bash

# Compile the MPI programs
mpicc -o create-sample-mp create-sample-mp.c -lm
mpicc -o analyze-mp analyze-mp.c -lm

# Run the program with different numbers of processors and capture timing
for num_procs in 1 2 4 8; do
    echo "Running with ${num_procs} processors"

    # Time the generation process
    echo "Generating files with ${num_procs} processors"
    mpirun -n ${num_procs} ./create-sample-mp 1000000000

    # Time the analysis process
    echo "Analyzing files with ${num_procs} processors"
    time mpirun -np ${num_procs} ./analyze-mp ${num_procs}
done

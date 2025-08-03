# One Billion Row Challenge in C with Parallel Processing

This repository contains a C-based solution to the [One Billion Row Challenge (1BRC)](https://www.morling.dev/blog/one-billion-row-challenge/), optimized for performance using parallel processing techniques (OpenMP/MPI). The goal is to calculate the min, mean, and max temperature values for weather stations from a text file containing one billion measurements as quickly as possible.

## Project Overview

This implementation consists of two main components:

1.  **`create-sample-mp.c`**: A parallel program to generate the one-billion-row dataset, distributed across 16 separate files (`measurements-0.txt` to `measurements-15.txt`).
2.  **`analyze-mp.c`**: A parallel program that reads and processes these measurement files to compute the required statistics (min, mean, max) for each weather station.

The original serial C implementation from [Danny Van Kooten's blog](https://www.dannyvankooten.com/blog/2024/1brc/) is included in the `c-example` directory for reference.

## How to Run

### Prerequisites

*   A C compiler that supports OpenMP (like GCC).
*   An MPI implementation (like Open MPI).
*   `make`

### 1. Compile the Programs

Navigate to the `c-example` directory to compile the original serial versions (optional):
```bash
cd c-example
make
cd ..
```

To compile the parallel versions in the root directory:
```bash
# Compile the data generator
gcc -o create-sample-mp create-sample-mp.c -fopenmp -lm

# Compile the data analyzer
gcc -o analyze-mp analyze-mp.c -fopenmp -lm
```
*(Note: The `-fopenmp` flag is for OpenMP. If using MPI, you would use `mpicc`)*

### 2. Generate the Dataset

Run the parallel data generator. The following command will create 1 billion rows of sample data across 16 files.

```bash
./create-sample-mp 1000000000
```
*(This will take some time and generate a significant amount of data. You can use a smaller number for testing.)*

### 3. Analyze the Data

Run the parallel analyzer. The `submit.sh` script is configured to run the analysis with 1, 2, 4, 8, and 16 threads/processes and record the runtime.

```bash
./submit.sh
```

The output will be saved in files named `results-p.txt`, where `p` is the number of threads/processes used.

## Results

The program outputs the min, mean, and max temperature values for each station in alphabetical order, like so:
```
{Abha=5.0/18.0/27.4, Abidjan=15.7/26.0/34.1, Abéché=12.1/29.4/35.6, ...}
```
Performance results can be observed by inspecting the execution times logged by the `submit.sh` script.

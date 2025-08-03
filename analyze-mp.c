#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_FILES 16
#define MAX_CITY_NAME_LENGTH 100
#define MAX_LINES 10000

// Structure to store city data
struct CityData {
    char city[MAX_CITY_NAME_LENGTH];
    int count;
    double min;
    double max;
    double sum;
};

struct CityData cityData[MAX_LINES];
int cityCount = 0;

void read_file(int fileIndex, struct CityData *localCityData, int *localCityCount) {
    char fileName[20];
    snprintf(fileName, sizeof(fileName), "measurements-%d.txt", fileIndex);

    FILE *file = fopen(fileName, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char city[MAX_CITY_NAME_LENGTH];
        double temperature;

        if (sscanf(line, "%[^;];%lf", city, &temperature) == 2) {
            int found = 0;
            for (int i = 0; i < *localCityCount; i++) {
                if (strcmp(localCityData[i].city, city) == 0) {
                    if (temperature < localCityData[i].min) localCityData[i].min = temperature;
                    if (temperature > localCityData[i].max) localCityData[i].max = temperature;
                    localCityData[i].sum += temperature;
                    localCityData[i].count++;
                    found = 1;
                    break;
                }
            }
            if (!found && *localCityCount < MAX_LINES) {
                strcpy(localCityData[*localCityCount].city, city);
                localCityData[*localCityCount].min = temperature;
                localCityData[*localCityCount].max = temperature;
                localCityData[*localCityCount].sum = temperature;
                localCityData[*localCityCount].count = 1;
                (*localCityCount)++;
            }
        }
    }

    fclose(file);
}

void merge_results(struct CityData *globalCityData, int *globalCityCount, struct CityData *localCityData, int localCityCount) {
    for (int i = 0; i < localCityCount; i++) {
        int found = 0;
        for (int j = 0; j < *globalCityCount; j++) {
            if (strcmp(globalCityData[j].city, localCityData[i].city) == 0) {
                if (localCityData[i].min < globalCityData[j].min) globalCityData[j].min = localCityData[i].min;
                if (localCityData[i].max > globalCityData[j].max) globalCityData[j].max = localCityData[i].max;
                globalCityData[j].sum += localCityData[i].sum;
                globalCityData[j].count += localCityData[i].count;
                found = 1;
                break;
            }
        }
        if (!found && *globalCityCount < MAX_LINES) {
            strcpy(globalCityData[*globalCityCount].city, localCityData[i].city);
            globalCityData[*globalCityCount].min = localCityData[i].min;
            globalCityData[*globalCityCount].max = localCityData[i].max;
            globalCityData[*globalCityCount].sum = localCityData[i].sum;
            globalCityData[*globalCityCount].count = localCityData[i].count;
            (*globalCityCount)++;
        }
    }
}

// Comparison function for qsort
int compare(const void *a, const void *b) {
    struct CityData *cityA = (struct CityData *)a;
    struct CityData *cityB = (struct CityData *)b;
    return strcmp(cityA->city, cityB->city);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0 || num_threads > NUM_FILES) {
        if (rank == 0) {
            fprintf(stderr, "Number of threads must be between 1 and %d\n", NUM_FILES);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    struct CityData localCityData[MAX_LINES];
    int localCityCount = 0;

    // Each process reads its corresponding file
    if (rank < num_threads) {
        read_file(rank, localCityData, &localCityCount);
    }

    // Gather results from all processes
    struct CityData *allCityData = NULL;
    int *allCityCounts = NULL;
    if (rank == 0) {
        allCityData = (struct CityData *)malloc(size * MAX_LINES * sizeof(struct CityData));
        allCityCounts = (int *)malloc(size * sizeof(int));
    }

    MPI_Gather(&localCityCount, 1, MPI_INT, allCityCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(localCityData, MAX_LINES * sizeof(struct CityData), MPI_BYTE, allCityData, MAX_LINES * sizeof(struct CityData), MPI_BYTE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int i = 0; i < size; i++) {
            merge_results(cityData, &cityCount, &allCityData[i * MAX_LINES], allCityCounts[i]);
        }

        // Sort the cityData array alphabetically by city name
        qsort(cityData, cityCount, sizeof(struct CityData), compare);

        // Write results to output file
        char outputFileName[20];
        snprintf(outputFileName, sizeof(outputFileName), "results-%d.txt", num_threads);
        FILE *outputFile = fopen(outputFileName, "w");
        if (!outputFile) {
            perror("Error opening output file");
            free(allCityData);
            free(allCityCounts);
            MPI_Finalize();
            return EXIT_FAILURE;
        }

        for (int i = 0; i < cityCount; i++) {
            fprintf(outputFile, "%s=%.1f/%.1f/%.1f\n",
                    cityData[i].city,
                    cityData[i].min,
                    cityData[i].sum / cityData[i].count,
                    cityData[i].max);
        }

        fclose(outputFile);
        free(allCityData);
        free(allCityCounts);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

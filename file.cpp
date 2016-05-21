#include "mpi.h"
#include <time.h>
#include <math.h>

int     ARRAY_SIZE = 40;

void    printArray(int *array, int size) {
        for (int i = 0; i < size; i++) {
                std::cout << array[i];
                if (i != size - 1)
                        std::cout << ", ";
        }
        std::cout << std::endl;
}

int     sum(int *array, int size) {
        int sum = 0;
        for (int i = 0; i < size; i++)
                sum += array[i];
        return sum;
}

double  sumDifferences(int *array, int size, double mean) {
        double sum = 0;
        for (int i = 0; i < size; i++) {
                sum += (array[i] - mean) * (array[i] - mean);
        }
        return sum;
}

int     coordinator(int world_size) {
        srand(1);
        int *array = new int[ARRAY_SIZE];
        int *partition = new int[ARRAY_SIZE / world_size];
        for (int i = 0; i < ARRAY_SIZE; i++) {
                array[i] = rand() % 51;
        }

        int partition_size = ARRAY_SIZE / world_size;

        MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Scatter(array, partition_size, MPI_INT, partition, partition_size,
                    MPI_INT, 0, MPI_COMM_WORLD);

        double average = (double)sum(partition, partition_size) / partition_size;
        std::cout << "\e[36mCoordinator\e[0m average is " << average << std::endl;

        double total_average = 0;
        MPI_Reduce(&average, &total_average, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        double overall_mean = total_average / world_size;

        MPI_Bcast(&overall_mean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double sum_difference = sumDifferences(partition, partition_size, overall_mean);
        double total_sum_difference = 0;
        MPI_Reduce(&sum_difference, &total_sum_difference, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        double standard_deviation = sqrt((total_sum_difference / ARRAY_SIZE));
        std::cout << "\e[34mArray used\e[0m : ";
        printArray(array, ARRAY_SIZE);
        std::cout << "\e[34mMean\e[0m : " << overall_mean << std::endl;
        std::cout << "\e[34mStandard deviation\e[0m  : " << standard_deviation << std::endl;

        delete array;
        delete partition;
}

int     participant(int world_rank) {

        int partition_size = 0;
        MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *partition = new int[partition_size];
        MPI_Scatter(NULL, partition_size, MPI_INT, partition, partition_size,
                    MPI_INT, 0, MPI_COMM_WORLD);

        double average = (double)sum(partition, partition_size) / partition_size;
        std::cout << "\e[35mParticipant\e[0m rank " << world_rank << " average is " << average << std::endl;

        MPI_Reduce(&average, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        double overall_mean = 0;
        MPI_Bcast(&overall_mean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double sum_difference = sumDifferences(partition, partition_size, overall_mean);
        MPI_Reduce(&sum_difference, NULL, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        delete partition;
}


int     main(int argc, char **argv) {
        int world_size, world_rank;

        if (argc == 2)
                ARRAY_SIZE = atoi(argv[1]);

        MPI_Init(&argc,&argv);
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        if (world_rank == 0) {
                coordinator(world_size);
        } else {
                participant(world_rank);
        }

        MPI_Finalize();

        return 0;
}

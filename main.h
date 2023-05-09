#ifndef MAIN_H
#define MAIN_H

#define MAX_PATH_SIZE 50
#define POPULATION_SIZE 100
#define ITERATION_COUNT 100000
#define ELITE_SIZE 5
#define MUTATION_RATE 0.01

typedef struct city{
    double x, y;
} City;

typedef struct path{
    unsigned cities[MAX_PATH_SIZE];
    double fitness;
    double distance;
} Path;

Path createIndividual(City *cityArray, int size);

double permutate(unsigned * individual, City *cityArray, int size);

void orderedCrossover(unsigned *child, unsigned *parent1, unsigned *parent2, int size);

void mutate(unsigned *individual, double mutationRate, int size);

int readFile(const char *str, City *cityArray);

double calculateDistance(unsigned * individual, City *cityArray, int size);

double randmm();



#endif
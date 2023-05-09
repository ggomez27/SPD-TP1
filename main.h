#ifndef MAIN_H
#define MAIN_H

#define MAX_PATH_SIZE 50
#define POPULATION_SIZE 400
#define ITERATION_COUNT 10000
#define ELITE_SIZE 10
#define MUTATION_RATE 0.01
#define MAX_LINE_LENGTH 100
#define NUM_THREAD 4



typedef struct city{
    double x, y;
} City;

typedef struct path{
    unsigned cities[MAX_PATH_SIZE];
    double distance;
} Path;

typedef struct ranking{
    unsigned index;
    double fitness;
} Ranking;

typedef struct dataStruct{
    Path * population;
    Path * pool;
    Ranking * ranking;
    int * selection;
} DataStruct;

typedef struct pThreadArg{
    int tid;
    DataStruct data;
} PThreadArg;

Path createIndividual();

double permutate(unsigned * individual);

void orderedCrossover(unsigned *child, unsigned *parent1, unsigned *parent2);

void mutate(unsigned *individual, double mutationRate);

int readFile(const char *str);

double calculateDistance(unsigned * individual);

double calculateFitness(double distance);

double randmm();

void sIterateGeneration(DataStruct data);

void pIterateGeneration(DataStruct data);

void printPath(Path array);

City cityArray[MAX_PATH_SIZE];
int populationSize = POPULATION_SIZE;
int eliteSize = ELITE_SIZE;
int iterationCount = ITERATION_COUNT;
double mutationRate = MUTATION_RATE;
int threadCount = NUM_THREAD;
int cityCount = -1;


#endif
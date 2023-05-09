#ifndef MAIN_H
#define MAIN_H

#define MAX_PATH_SIZE 50
#define POPULATION_SIZE 400
#define ITERATION_COUNT 10000
#define ELITE_SIZE 10
#define MUTATION_RATE 0.01
#define MAX_LINE_LENGTH 100
#define NUM_THREAD 4

// Representacion de una Ciudad como un par de coordenadas (x,y)
typedef struct city{
    double x, y;
} City;

// Representacion de un Camino como un Arreglo de indices de Ciudades, y su distancia total
typedef struct path{
    unsigned cities[MAX_PATH_SIZE];
    double distance;
} Path;

// Estructura utilizada para ordenar los Caminos en base a su fitness
typedef struct ranking{
    unsigned index;
    double fitness;
} Ranking;

// Estructura de Datos para el algoritmo genetico
typedef struct dataStruct{
    Path * population;
    Path * pool;
    Ranking * ranking;
    int * selection;
} DataStruct;

// Estructura de Datos del argumento pasado cuando se llama a un nuevo pThread
typedef struct pThreadArg{
    int tid;
    DataStruct data;
} PThreadArg;

// Funcion que crea un individuo de forma randomica
Path createIndividual();

// Crea una permutacion de los indices de las ciudades
double permutate(unsigned * individual);

// Realiza el ordered crossover entre dos padres y asigna el resultado a child
void orderedCrossover(unsigned *child, unsigned *parent1, unsigned *parent2);

// Realiza una mutacion swap de acuerdro a la probabilidad indicada por mutationRate
void mutate(unsigned *individual, double mutationRate);

// Lee un archivo y almacena la lista de ciudades en la variable global cityArray
int readFile(const char *str);

// Calcula la distancia total de una lista de ciudades
double calculateDistance(unsigned * individual);

// Calcula el fitness en base a una distancia obtenida
double calculateFitness(double distance);

// Elije al azar un numero double entre 0 y 1
double randmm();

// Version en Serie del Algoritmo Genetico
void sIterateGeneration(DataStruct data);

// Version en Paralelo del Algoritmo Genetico
void pIterateGeneration(DataStruct data);

// Imprime un camino y su distancia
void printPath(Path array);

// Representacion de las ciudades en un arreglo
City cityArray[MAX_PATH_SIZE];

int populationSize = POPULATION_SIZE;
int eliteSize = ELITE_SIZE;
int iterationCount = ITERATION_COUNT;
double mutationRate = MUTATION_RATE;
int threadCount = NUM_THREAD;
int cityCount = -1;


#endif
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

int main(int argc, char *argv[]){
    // timer start
	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

    puts("Reading file...");
    cityCount = readFile("data/dj38.tsp");

    if(cityCount == -1)
        return -1;
    
    printf("%i cities have been loaded\n", cityCount);

    if(eliteSize >= populationSize)
        puts("El tamaño definido para la seleccion por Elitismo es mayor o igual al tamaño de poblacion. La seleccion tomara el total de la poblacion.");
    
    // Memoria que se compartira entre los hilos. Los hilos trabajan siempre en los indices ThreadID + i * ThreadCount, evitando asi colisiones.
    
    Path population[populationSize];
    Path pool[populationSize];
    Ranking ranking[populationSize];
    int selection[populationSize];

    // Estructura de datos para la memoria compartida.
    DataStruct data;
    data.pool = NULL;
    data.population = population;
    data.ranking = ranking;
    data.selection = selection;
    

    puts("Starting GA");
    unsigned iteration = 0;
    // Comienzo del Algoritmo Genetico
    while((iteration++)<iterationCount) {

        // pIterateGeneration ejecuta en paralelo, sIterateGeneration en serie.
        pIterateGeneration(data);

        // Luego de hacer el join de los hilos, se prepara el pool para criar la siguiente generacion.
        for(int i = 0; i < populationSize; i++){
            pool[i]=population[selection[i]];
        }

        // Esta asignacion solo es relevante en la primera iteracion, para indicar al algoritmo que ya se cuenta con una poblacion para procrear.
        if(iteration==1)
            data.pool=pool;

    }

    double min = population[0].distance;
    unsigned minIndex = 0;
    for(int i = 1; i < populationSize; i++){
        if(population[i].distance<min){
            min=population[i].distance;
            minIndex=i;
        }
    }

    // Imprime el resultado.
    puts("Done.\nResults:");
    printPath(population[minIndex]);


    struct timespec ts_stop;
	clock_gettime(CLOCK_MONOTONIC, &ts_stop);
	double start = (double)ts_start.tv_sec + (double)ts_start.tv_nsec/1000000000.0;
	double stop = (double)ts_stop.tv_sec + (double)ts_stop.tv_nsec/1000000000.0;
	double elapsed = (stop - start);

    // display results
	printf ("Time = %f\n",elapsed);

    return 0;
    
}


int readFile(const char *str) {
    char line[MAX_LINE_LENGTH];
    FILE *myfile = fopen(str, "r");

    int value;
    if (myfile != NULL) {
        while (fgets(line, MAX_LINE_LENGTH, myfile) != NULL) {
            if (strncmp(line, "DIMENSION", 9) == 0) {
                value = atoi(&line[12]);
                // printf("value: %d\n", value);
                if(value>MAX_PATH_SIZE){
                    puts("Dimension is too big");
                    return -1;
                }
                continue;
            }
            if (strncmp(line, "NODE_COORD_SECTION", 18) == 0)
                break;
        }
        int index = 0;
        while (fgets(line, MAX_LINE_LENGTH, myfile) != NULL && line[0] != 'E') {
            char *s = line;
            char *delimiter = " ";
            char *token;
            char **res = (char**) malloc(sizeof(char*) * 3);

            int i = 0;
            while ((token = strtok(s, delimiter)) != NULL) {
                s = NULL;
                res[i++] = token;
            }
            City c;
            c.x = strtof(res[1], NULL);
            c.y = strtof(res[2], NULL);
            cityArray[index].x = c.x;
            cityArray[index].y = c.y;
            // printf("Ciudad %d: (%.5f, %.5f)\n", index, cityArray[index].x, cityArray[index].y);
            free(res);
            index++;
          }
          fclose(myfile);
        }
    else {
        printf("Unable to open file\n");
        return -1;
    }

    return value;
    }


double randmm(){
    return (double)rand() / ((double) RAND_MAX);
}

double distanceBetween(City a, City b){
    return sqrt(pow(a.x - b.x,2.0) + pow(a.y - b.y,2.0));
}

double calculateDistance(unsigned * individual){
    unsigned i, a, b;
    double distance = 0.0;
    for(i = 0; i < cityCount; i++){
        a = individual[i], b = individual[(i+1)%cityCount];
        distance += distanceBetween(cityArray[a],cityArray[b]);
    }
    return distance;
}

double calculateFitness(double distance){
    return  1.0 / distance;
}

Path createIndividual(){
    Path individual;
    individual.distance = permutate(individual.cities);
    return individual;
}

double permutate(unsigned * individual){
    unsigned i, a, b;
    double distance = 0;
    char values[cityCount];
    for(i = 0; i < cityCount; i++)
        values[i] = 0;
    for(i = 0; i < cityCount; i++){
        unsigned index;
        do{
            index = rand()%cityCount;
        } while(values[index]);
        individual[i] = index;
        values[index] = 1;

        if(i > 0){
            a = individual[i-1], b = individual[i];
            distance += distanceBetween(cityArray[a],cityArray[b]);
        }
    }
    a = individual[0], b = individual[cityCount-1];
    return distance + distanceBetween(cityArray[a],cityArray[b]);
}

void orderedCrossover(unsigned *child, unsigned *parent1, unsigned *parent2){
    
    // Rango del subArray desde 0 hasta size-1 inclusivo.
    unsigned subArraySize = rand()%(cityCount);
    unsigned subArrayPosition = rand()%(cityCount-subArraySize);
    unsigned i;
    char values[cityCount];
    for(i = 0; i < cityCount; i++)
        values[i] = 0;

    // Copiar el subArray del parent 1 en su misma posicion
    for(i = 0; i < subArraySize; i++){
        child[subArrayPosition + i] = parent1[subArrayPosition + i];
        values[parent1[subArrayPosition + i]] = 1;
    }

    int j = 0;
    for(i = 0; i < cityCount; i++){
        // Saltar los elementos ya copiados
        if(subArraySize != 0 && i == subArrayPosition){
            i+=subArraySize-1;
            continue;
        }
        while(values[parent2[j]])
            j++;
        child[i]=parent2[j];
        values[parent2[j]]=1;
        j++;
    }
}

void mutate(unsigned *individual, double mutationRate){
    unsigned index;
    for(index = 0; index < cityCount; index++){
        if(randmm() < mutationRate){
            // No se garantiza que la mutacion sea efectiva: puede hacer un swap en un mismo lugar.
            unsigned swapIndex = rand()%cityCount;
            unsigned aux = individual[index];
            individual[index] = individual[swapIndex];
            individual[swapIndex] = aux;
        }
    }
}

/**
 * Esta funcion ejecuta el algoritmo genetico casi completamente. Lo unico que no realiza, es la copia de los individuos
 * seleccionados de la poblacion al pool. Esto debe realizarse fuera de esta funcion, porque al paralelizar esta funcion,
 * la seleccion se finaliza recien al realizar el join (cada hilo ya escogio sus mejores candidatos).
*/
void _IterateGeneration(DataStruct arg, int itStart, int itIncrement){
    Path * population = arg.population;
    Path * pool = arg.pool;
    Ranking * ranking = arg.ranking;
    int * selection = arg.selection;
    
    // Si pool es nulo, debo crear mi poblacion. 
    if(!pool){
        for(int i = itStart; i < populationSize; i+=itIncrement)
            population[i] = createIndividual();
    }
    else{ 
    // Inicio del Breeding.
        for(int i = itStart; i < populationSize; i+=itIncrement){
            if(i<eliteSize){
                population[i]=pool[i];
                continue;
            }
            unsigned parent = i-eliteSize;
            unsigned breeder = populationSize+eliteSize-i-1;
            if(parent!=breeder)
                orderedCrossover(population[i].cities,
                    pool[parent].cities,
                    pool[breeder].cities);
            else population[i]=pool[parent];
            mutate(population[i].cities,mutationRate);
            population[i].distance=calculateDistance(population[i].cities);
        }
    }

    // Inicio del Ranking
    char values[populationSize];
    for(int i = itStart; i < populationSize; i+=itIncrement)
        values[i] = 0;
    for(int i = itStart; i < populationSize; i+=itIncrement){
        unsigned maxIndex=0;
        double maxFitness = -1.0;
        for(int j = itStart; j < populationSize; j+=itIncrement){
            double fitness = calculateFitness(population[j].distance);
            if(values[j] == 0 && fitness > maxFitness)
                maxIndex=j, maxFitness=fitness;
        }
        ranking[i].index=maxIndex, ranking[i].fitness=maxFitness, values[maxIndex]=1;
    }

    // Inicio de Seleccion
    double sum = 0.0;
    double perc[populationSize];
    double cumulativeSum = 0.0;

    // Sumatoria de los valores de Fitness.
    for(int i = itStart; i < populationSize; i+=itIncrement)
        sum += ranking[i].fitness;

    // Determinacion de probabilidad para la Seleccion. 
    for(int i = itStart; i < populationSize; i+=itIncrement){
        unsigned index = ranking[i].index;
        cumulativeSum += ranking[index].fitness;
        perc[index] = cumulativeSum / sum;
    }
    
    // Se elige al azar de la poblacion, basado en las probabilidades determinadas anteriormente.
    for(int i = itStart; i < populationSize; i+=itIncrement){
        if(i<eliteSize){
            selection[i] = ranking[i].index;
            continue;
        }
        double pick = randmm();
        int j;
        for(j = itStart; j < populationSize; j+=itIncrement){
            unsigned index = ranking[j].index;
            if(pick <= perc[index]){
                selection[i]=index;
                
                break;
            }
        }
    }
}

void sIterateGeneration(DataStruct data){
    _IterateGeneration(data,0,1);
}

void * _pIterateGeneration(void * arg){
    PThreadArg pTArg = *((PThreadArg*) arg);
    _IterateGeneration(pTArg.data,pTArg.tid,threadCount);
}

void pIterateGeneration(DataStruct data){
    PThreadArg arg[threadCount];
    pthread_t tids[threadCount];

    for(int tid = 0; tid < threadCount; ++tid) {
        arg[tid].tid=tid;
        arg[tid].data=data;
        pthread_create(&tids[tid], NULL, _pIterateGeneration, &arg[tid]);
    }

    for (int tid = 0; tid < threadCount; ++tid) {
		pthread_join(tids[tid], NULL);
	}

}

void printPath(Path array){
    int i;
    printf("Path = ");
    for(i=0; i<cityCount; i++)
        printf("%u ", array.cities[i]+1);
    printf("\nPath Distance = %lf\n", array.distance);
}
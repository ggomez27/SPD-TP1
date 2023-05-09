#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 100

void imprimir(unsigned *array, int size){
    int i;
    for(i=0; i<size; i++)
        printf("%u ", array[i]+1);
    puts("");
}

int comp(const void * elem1, const void * elem2){
    Path f = *((Path*)elem1);
    Path s = *((Path*)elem2);
    if(f.fitness < s.fitness) return 1;
    if(f.fitness > s.fitness) return -1;
    return 0;
}

int main(int argc, char *argv[]){
    int i;
    
    City cityArray[MAX_PATH_SIZE];
    
    int cityCount = readFile("data/wi29.tsp", cityArray);

    if(cityCount == -1)
        return -1;
    
    printf("cityCount= %u\n", cityCount);
        
    unsigned iteration = 0;
    int populationSize = POPULATION_SIZE;
    int eliteSize = ELITE_SIZE;
    int iterationCount = ITERATION_COUNT;
    double mutationRate = MUTATION_RATE;
    
    if(eliteSize>= populationSize)
        puts("El tamaño definido para la seleccion por Elitismo es mayor al tamaño de poblacion. La seleccion tomara el total de la poblacion.");
    
    

    //Crear populacion y calcular fitness basado en 1/distancia total (paralelizable y sin memoria compartida)
    Path population[populationSize];
    for(i = 0; i < populationSize; i++){
        population[i] = createIndividual(cityArray,cityCount);
    }

    double minDist = 0;
    // Comienzo del Algoritmo Genetico
    while((iteration++)<iterationCount) {
        
        // Ranking y Seleccion
        qsort(population, sizeof(population)/sizeof(*population),sizeof(*population),comp);
        
        static int val = 0;
        if(val==0){
            minDist=population[0].distance;
            printf("Current min: %lf [%u]\n", minDist, iteration);
            val=1;
        }
        else{
            if(population[0].distance<minDist){
                minDist=population[0].distance;
            printf("Current min: %lf [%u]\n", minDist, iteration);
            }
        }
        // Suma de todos los valores de Fitness. Paralelizable con memoria compartida
        double sum = 0;
        for(i = 0; i < populationSize; i++)
            sum += population[i].fitness;

        double perc[populationSize];
        double cumulativeSum = 0.0;

        // Si la suma cumulativa se calculara y almacenara por separado (no paralelizable), lo siguiente seria paralelizable
        for(i = 0; i < populationSize; i++){
            cumulativeSum += population[i].fitness;
            perc[i] = cumulativeSum / sum;
        }

        // Seleccion. Paralelizable sin memoria compartida
        unsigned selection[populationSize];
        for(i = 0; i < populationSize; i++){
            if(i<eliteSize){
                selection[i] = i;
                continue;
            }
            double pick = randmm();
            int j;
            for(j = 0; j < populationSize; j++){
                if(pick <= perc[j]){
                    selection[i]=j;
                    break;
                }
            }
        }

        // 

        // Breeding. Paralelizable sin memoria compartida
        Path newPopulation[populationSize];
        for(i = 0; i < populationSize; i++){
            if(i<eliteSize){
                newPopulation[i]=population[i];
                continue;
            }
            unsigned breeder = populationSize+eliteSize-i-1;
            if(i!=breeder)
                orderedCrossover(newPopulation[i].cities,
                    population[i].cities,
                    population[breeder].cities,
                    cityCount);
            else newPopulation[i]=population[i];
            mutate(newPopulation[i].cities,mutationRate, cityCount);
            newPopulation[i].distance=calculateDistance(newPopulation[i].cities,cityArray,cityCount);
            newPopulation[i].fitness=1.0 / newPopulation[i].distance;
        }
        

        // Paralelizable sin memoria compartida
        for(i = 0; i < populationSize; i++){
            population[i]=newPopulation[i];
        }

    };

    double min = population[0].distance;
    unsigned minIndex = 0;
    for(i = 1; i < populationSize; i++){
        if(population[i].distance<min){
            min=population[i].distance;
            minIndex=i;
        }
    }

    imprimir(population[minIndex].cities, cityCount);
    printf("Distancia minima: %lf\n", population[minIndex].distance);

    return 0;
}


int readFile(const char *str, City *cityArray) {
    char line[MAX_LINE_LENGTH];
    FILE *myfile = fopen(str, "r");

    int value;
    if (myfile != NULL) {
        while (fgets(line, MAX_LINE_LENGTH, myfile) != NULL) {
            if (strncmp(line, "DIMENSION", 9) == 0) {
                value = atoi(&line[12]);
                printf("value: %d\n", value);
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
            printf("Ciudad %d: (%.5f, %.5f)\n", index, cityArray[index].x, cityArray[index].y);
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

double calculateDistance(unsigned * individual, City *cityArray, int size){
    unsigned i, a, b;
    double distance = 0.0;
    for(i = 0; i < size; i++){
        a = individual[i], b = individual[(i+1)%size];
        distance += distanceBetween(cityArray[a],cityArray[b]);
    }
    return distance;
}

Path createIndividual(City *cityArray, int size){
    Path individual;
    individual.distance = permutate(individual.cities,cityArray,size);
    individual.fitness= 1.0 / individual.distance;
    return individual;
}

double permutate(unsigned * individual, City *cityArray, int size){
    unsigned i, a, b;
    double distance = 0;
    char values[size];
    for(i = 0; i < size; i++)
        values[i] = 0;
    for(i = 0; i < size; i++){
        unsigned index;
        do{
            index = rand()%size;
        } while(values[index]);
        individual[i] = index;
        values[index] = 1;

        if(i > 0){
            a = individual[i-1], b = individual[i];
            distance += distanceBetween(cityArray[a],cityArray[b]);
        }
    }
    a = individual[0], b = individual[size-1];
    return distance + distanceBetween(cityArray[a],cityArray[b]);
}

void orderedCrossover(unsigned *child, unsigned *parent1, unsigned *parent2, int size){
    
    // Rango del subArray desde 0 hasta size-1 inclusivo.
    unsigned subArraySize = rand()%(size);
    unsigned subArrayPosition = rand()%(size-subArraySize);
    unsigned i;
    char values[size];
    for(i = 0; i < size; i++)
        values[i] = 0;

    // Copiar el subArray del parent 1 en su misma posicion
    for(i = 0; i < subArraySize; i++){
        child[subArrayPosition + i] = parent1[subArrayPosition + i];
        values[parent1[subArrayPosition + i]] = 1;
    }

    int j = 0;
    for(i = 0; i < size; i++){
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

void mutate(unsigned *individual, double mutationRate, int size){
    unsigned index;
    for(index = 0; index < size; index++){
        if(randmm() < mutationRate){
            // No se garantiza que la mutacion sea efectiva: puede hacer un swap en un mismo lugar.
            unsigned swapIndex = rand()%size;
            unsigned aux = individual[index];
            individual[index] = individual[swapIndex];
            individual[swapIndex] = aux;
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define VECTOR_SIZE 200000000

// Função que simula uma carga de trabalho pesada
void heavy_work(double *vector, int start, int end) {
    for (int i = start; i < end; ++i) {
        vector[i] = sin(vector[i]) * cos(vector[i]) + sqrt(vector[i]);
    }
}

int main() {
    double *vector = (double *) malloc(VECTOR_SIZE * sizeof(double));
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector[i] = (double) i;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    heavy_work(vector, 0, VECTOR_SIZE);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Versao sequencial executou em %f segundos\n", time_spent);

    // Para verificar se o cálculo foi feito, descomente a linha abaixo:
    // printf("Resultado de verificacao: vector[10] = %f\n", vector[10]);

    free(vector);
    return 0;
}

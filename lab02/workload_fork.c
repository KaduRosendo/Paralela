<<<<<<< HEAD
=======
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define VECTOR_SIZE 200000000 // Tamanho do vetor 
#define NUM_PROCESSES 4 //Número de porcessos 

//Função que simula uma carga de trabalho pesada
void heavy_work(double* vector, int start, int end) {
    for (int i = start; i < end; i++) {
        vector[i] = sin(vector[i]) * cos(vector[i]) + sqrt(vector[i]);
    }
}

int main() {
    double* vector = (double*)malloc(VECTOR_SIZE * sizeof(double));
    for(int i = 0; i < VECTOR_SIZE; i++) {
        vector[i] = (double)i;

    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int chunck_size = VECTOR_SIZE / NUM_PROCESSES;

    //Criação de multiplos processos
    for(int i = 0; i < NUM_PROCESSES; i++){
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork falhou");
        }else if (pid == 0) {
            int start_idx = i * chunck_size;
            int end_idx = (i + 1) * chunck_size;
            if (i == NUM_PROCESSES - 1){
                end_idx = VECTOR_SIZE;
            }

            heavy_work(vector, start_idx, end_idx);
            exit(0);
            
        }
    }

    //Pai espera todos os filhos 
    for (int i = 0; i < NUM_PROCESSES; i++){
        wait(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; //Calcula o tempo gasto

    printf("Versão paralela executando em %f segundos\n", time_spent);

    printf("Resultado de verificação: vector[10] = %f\n", vector[10]);

    free(vector);
    return 0;

}
>>>>>>> f3da9ae (Subindo atualizações do Labs)


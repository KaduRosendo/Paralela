#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


#define NUM_PROCESSES 4 //número de processos
#define VECTOR_SIZE 1000 // tamanho total do vetor

int main() {
    // variáveis
    int vector[VECTOR_SIZE];
    int pipes[NUM_PROCESSES][2]; // Array de pipes
    pid_t pids[NUM_PROCESSES];   // Array de PIDs
    int total_sum = 0; 

    
    int verification_sum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector[i] = i + 1; 
        verification_sum += vector[i]; // Soma de verificação sequencial
    }

    // Calcula o tamanho do segmento que cada processo filho irá somar
    int segment_size = VECTOR_SIZE / NUM_PROCESSES;

    // Laço para criar os processos filhos
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Cria um processo filho usando fork
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // código para o processo filho
        if (pids[i] == 0) {
            // Fecha a ponta de leitura 
            close(pipes[i][0]);

            // Fecha os pipes herdados dos outros processos que não serão usados
            for (int j = 0; j < NUM_PROCESSES; j++) {
                if (i != j) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            
            // Calcula os índices de início e fim para o segmento deste filho
            int start_index = i * segment_size;
            int end_index = start_index + segment_size;
            int local_sum = 0;

            // Calcula a soma parcial do seu segmento
            for (int k = start_index; k < end_index; k++) {
                local_sum += vector[k];
            }

            // Envia a soma parcial
            if (write(pipes[i][1], &local_sum, sizeof(int)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            
            // Fecha a ponta de escrita do seu pipe após o uso
            close(pipes[i][1]);
            
            // Termina o processo filho com sucesso
            exit(EXIT_SUCCESS);
        }
        // Bloco de código para o processo pai 
        else {
            // O pai fecha a ponta de escrita
            close(pipes[i][1]);
        }
    }

    // Laço para ler as somas parciais de cada filho
    printf("Processo pai aguardando as somas parciais dos filhos...\n");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int partial_sum; 
        if (read(pipes[i][0], &partial_sum, sizeof(int)) == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        printf("Pai recebeu a soma parcial %d do filho %d\n", partial_sum, i);
        
        // Acumula a soma parcial na variável total
        total_sum += partial_sum;

        // Fecha a ponta de leitura do pipe após o uso
        close(pipes[i][0]);
    }

    // Espera todos os processos filhos terminarem
    printf("Processo pai esperando todos os filhos terminarem...\n");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    // Imprime o resultado final
    printf("\n--- Resultados ---\n");
    printf("Soma total calculada (paralela): %d\n", total_sum);
    printf("Soma de verificação (sequencial): %d\n", verification_sum);

    return 0;
}

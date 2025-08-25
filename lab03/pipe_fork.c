#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Define o número de processos filhos a serem criados
#define NUM_PROCESSES 4
// Define o tamanho total do vetor
#define VECTOR_SIZE 1000

int main() {
    // Declaração do vetor e outras variáveis
    int vector[VECTOR_SIZE];
    int pipes[NUM_PROCESSES][2]; // Array de pipes, um para cada processo filho
    pid_t pids[NUM_PROCESSES];   // Array para armazenar os PIDs dos filhos
    int total_sum = 0;           // Usando int para a soma total

    // 1. Preenche o vetor com uma sequência de números (1, 2, 3, ..., 1000)
    printf("Inicializando o vetor de %d posições com a sequência 1, 2, 3...\n", VECTOR_SIZE);
    int verification_sum = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vector[i] = i + 1; // O vetor agora recebe a sequência i + 1
        verification_sum += vector[i]; // Soma de verificação sequencial
    }

    // Calcula o tamanho do segmento que cada processo filho irá somar
    int segment_size = VECTOR_SIZE / NUM_PROCESSES;

    // Laço para criar os processos filhos
    for (int i = 0; i < NUM_PROCESSES; i++) {
        // 3. Cria um pipe para o i-ésimo filho
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // 2. Cria um processo filho usando fork
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // 5. Bloco de código para o processo filho
        if (pids[i] == 0) {
            // Fecha a ponta de leitura do seu próprio pipe, pois só vai escrever
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
            int local_sum = 0; // Usando int para a soma local

            // Calcula a soma parcial do seu segmento
            for (int k = start_index; k < end_index; k++) {
                local_sum += vector[k];
            }

            // Envia a soma parcial (int) para o processo pai através do pipe
            if (write(pipes[i][1], &local_sum, sizeof(int)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            
            // Fecha a ponta de escrita do seu pipe após o uso
            close(pipes[i][1]);
            
            // Termina o processo filho com sucesso
            exit(EXIT_SUCCESS);
        }
        // 6. Bloco de código para o processo pai (dentro do laço)
        else {
            // O pai fecha a ponta de escrita do pipe do filho recém-criado.
            // Esta é a correção crucial.
            close(pipes[i][1]);
        }
    }

    // Laço para ler as somas parciais de cada filho
    printf("Processo pai aguardando as somas parciais dos filhos...\n");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int partial_sum; // Usando int para a soma parcial
        // Lê a soma parcial do pipe do i-ésimo filho
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
    
    // 7. Imprime o resultado final
    printf("\n--- Resultados ---\n");
    printf("Soma total calculada (paralela): %d\n", total_sum);
    printf("Soma de verificação (sequencial): %d\n", verification_sum);

    // Verifica se os resultados são iguais
    if (total_sum == verification_sum) {
        printf("VERIFICAÇÃO: O resultado está correto!\n");
    } else {
        printf("VERIFICAÇÃO: O resultado está incorreto!\n");
    }

    return 0;
}

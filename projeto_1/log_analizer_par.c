#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_LINE_LENGTH 1024

// 4. Estrutura de dados global para armazenar as estatísticas
struct Stats {
    long long errors404;
    long long total_bytes;
};

// Estrutura para passar argumentos para cada thread
struct ThreadArgs {
    char **lines;          // Ponteiro para o array de todas as linhas
    long long start_index; // Linha inicial para esta thread
    long long end_index;   // Linha final (exclusivo) para esta thread
};

// Variáveis globais
struct Stats global_stats = {0, 0};
char **all_lines = NULL;

// 5. Mutex global para proteger a estrutura de estatísticas
pthread_mutex_t stats_mutex;

// A função que cada thread irá executar
void *process_lines_chunk(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;

    // Variáveis locais para diminuir a contenção do mutex (abordagem opcional, mas boa prática)
    // Para seguir estritamente o requisito 6, o código abaixo atualiza diretamente o global.
    
    for (long long i = args->start_index; i < args->end_index; i++) {
        char *line = args->lines[i];
        int status_code;
        long bytes;

        // Tenta extrair o código de status e os bytes
        if (sscanf(line, "%*s %*s %*s [%*[^]]] \"%*[^\"]\" %d %ld", &status_code, &bytes) == 2) {
            if (status_code == 404) {
                // 6. Trava, atualiza e destrava
                pthread_mutex_lock(&stats_mutex);
                global_stats.errors404++;
                pthread_mutex_unlock(&stats_mutex);
            } else if (status_code == 200) {
                // 6. Trava, atualiza e destrava
                pthread_mutex_lock(&stats_mutex);
                global_stats.total_bytes += bytes;
                pthread_mutex_unlock(&stats_mutex);
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    // 1. Receber o número de threads como argumento
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_de_threads> <arquivo_de_log>\n", argv[0]);
        return 1;
    }
    int num_threads = atoi(argv[1]);
    const char *filename = argv[2];
    if (num_threads <= 0) {
        fprintf(stderr, "O número de threads deve ser maior que zero.\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Não foi possível abrir o arquivo");
        return 1;
    }

    // 2. Ler todas as linhas do arquivo para um array na memória
    printf("Fase 1: Lendo o arquivo para a memória...\n");
    long long line_count = 0;
    char buffer[MAX_LINE_LENGTH];

    // Primeiro, conta as linhas para alocar a memória necessária
    while (fgets(buffer, sizeof(buffer), file)) {
        line_count++;
    }
    rewind(file); // Volta para o início do arquivo

    // Aloca memória para o array de ponteiros de linha
    all_lines = malloc(sizeof(char *) * line_count);
    if (!all_lines) {
        perror("Falha ao alocar memória para as linhas");
        fclose(file);
        return 1;
    }

    // Agora, lê as linhas e as armazena
    long long current_line = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        all_lines[current_line] = strdup(buffer); // strdup aloca memória e copia a string
        if (!all_lines[current_line]) {
            perror("Falha ao duplicar a string da linha");
            // Liberar memória já alocada... (omitido por simplicidade)
            return 1;
        }
        current_line++;
    }
    fclose(file);
    printf("%lld linhas lidas para a memória.\n", line_count);
    
    // Inicia a contagem do tempo APÓS a leitura do arquivo
    clock_t start_time = clock();

    // Inicializa o mutex
    if (pthread_mutex_init(&stats_mutex, NULL) != 0) {
        perror("Falha ao inicializar o mutex");
        return 1;
    }

    pthread_t threads[num_threads];
    struct ThreadArgs thread_args[num_threads];

    // 3. Dividir o trabalho
    long long lines_per_thread = line_count / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].lines = all_lines;
        thread_args[i].start_index = i * lines_per_thread;
        // A última thread pega todas as linhas restantes
        thread_args[i].end_index = (i == num_threads - 1) ? line_count : (i + 1) * lines_per_thread;

        // 6. Disparar as N threads
        if (pthread_create(&threads[i], NULL, process_lines_chunk, &thread_args[i]) != 0) {
            perror("Falha ao criar a thread");
            return 1;
        }
    }

    // 7. A thread principal deve esperar todas as threads terminarem
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Falha ao esperar a thread");
            return 1;
        }
    }

    // Destrói o mutex após o uso
    pthread_mutex_destroy(&stats_mutex);

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // 8. Imprimir o tempo de execução e as estatísticas finais
    printf("\n--- Análise Paralela Concluída ---\n");
    printf("Tempo de processamento (paralelo): %.4f segundos\n", elapsed_time);
    printf("Requisições com erro 404: %lld\n", global_stats.errors404);
    printf("Total de bytes transferidos (status 200): %lld bytes\n", global_stats.total_bytes);

    // Liberar toda a memória alocada
    for (long long i = 0; i < line_count; i++) {
        free(all_lines[i]);
    }
    free(all_lines);

    return 0;
}

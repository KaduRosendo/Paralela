#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

struct Stats {
    long long errors404;
    long long total_bytes;
};

struct ThreadArgs {
    char **lines;
    long long start_index;
    long long end_index;
};

struct Stats global_stats = {0, 0};
pthread_mutex_t stats_mutex;

void *process_line_chunck(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    struct Stats local_stats = {0, 0};

    for (long long i = args->start_index; i < args->end_index; i++) {
        char *line = args->lines[i];

        char *quote_ptr = strstr(line, "\" ");
        if(quote_ptr){
            int status_code;
            long bytes;
            if (sscanf(quote_ptr + 2, "%d %ld", &status_code, &bytes) == 2) {
                if (status_code == 404){
                    local_stats.errors404++;
                } else if (status_code == 200) {
                    local_stats.total_bytes += bytes;
                }
            }
        }
    }

    pthread_mutex_lock(&stats_mutex);
    global_stats.errors404 += local_stats.errors404;
    global_stats.total_bytes += local_stats.total_bytes;
    pthread_mutex_unlock(&stats_mutex);
    
    return NULL;
}

void run_analysis (int num_threads, const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nao foi possivel abrir o arquivo!!");
        return;
    }

    char **all_lines = NULL;
    char *line_buffer = NULL;
    size_t line_buffer_size = 0;
    long long line_count = 0;
    long long line_capacity = 0;

    while (getline(&line_buffer, &line_buffer_size, file) != -1) {
        if (line_count >= line_capacity) {
            line_capacity = (line_capacity == 0) ? 1024 : line_capacity * 2;
            char**new_lines = realloc(all_lines, line_capacity * sizeof(char *));
            if (!new_lines) {
                perror("Falha ao realocar memoria");
                exit(EXIT_FAILURE);
            }
            all_lines = new_lines;
        }
        all_lines[line_count++] = strdup(line_buffer);
    }
    free(line_buffer);
    fclose(file);
    printf("%lld linhas lidas para a memória\n", line_count);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_mutex_init(&stats_mutex, NULL);
    pthread_t threads[num_threads];
    struct ThreadArgs thread_args[num_threads];

    long long lines_per_thread = line_count / num_threads;
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].lines = all_lines;
        thread_args[i].start_index = i * lines_per_thread;
        thread_args[i].end_index = (i == num_threads - 1) ? line_count : (i + 1) * lines_per_thread;
        pthread_create(&threads[i], NULL, process_line_chunck, &thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&stats_mutex);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n--- Análise Paralela ---\n");
    printf("Tempo de processamento (paralelo): %.4f segundos\n", time_spent);
    printf("Requisições com erro 404: %lld\n", global_stats.errors404);
    printf("Total de bytes transferidos (status 200): %lld bytes\n", global_stats.total_bytes);

    for (long long i = 0; i < line_count; i++) {
        free(all_lines[i]);
    }
    free(all_lines);
}

int main () {
    const int NUM_THREADS = 4;
    const char *log_file = "access_log_large.txt";

    run_analysis(NUM_THREADS, log_file);

    return 0;
}






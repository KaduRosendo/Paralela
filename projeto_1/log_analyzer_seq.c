
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void analyze_log (const char *filename){
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Não foi possivel abrir o arquiuvo");
        return;
    }

    long long count_404 = 0;
    long long total_bytes = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        int status_code;
        long bytes;

        char *quote_ptr = strstr ( line , "\" ") ;
        if (quote_ptr) {
            if (sscanf(quote_ptr + 2, "%d %ld", &status_code, &bytes) == 2) {
                if (status_code == 404) {
                    count_404++;
                }else if (status_code == 200) {
                    total_bytes += bytes;
                }
            }
        }
    }


    free(line);
    fclose(file);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n--- Análise Sequencial  ---\n");
    printf("Requisições com erro 404: %lld\n", count_404);
    printf("Total de bytes transferidos (status 200): %lld bytes\n", total_bytes);
    printf("Tempo total de execução: %.4f segundos\n", elapsed_time);   
}

int main() {
    const char *log_file = "access_log_large.txt";
    analyze_log(log_file);
    return 0;
}

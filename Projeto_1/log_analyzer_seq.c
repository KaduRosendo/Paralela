
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 1024

void analyze_log(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Não foi possível abrir o arquivo");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int count_404 = 0;
    long total_bytes = 0;
    int line_count = 0;
    clock_t start_time = clock();

    while (fgets(line, sizeof(line), file)) {
        int status_code, bytes;

        if (sscanf(line, "%*s %*s %*s [%*[^]]] \"%*s %*s %*s\" %d %d", &status_code, &bytes) == 2) {
            if (status_code == 404) {
                count_404++;
            } else if (status_code == 200) {
                total_bytes += bytes;
            }
        }

        line_count++;
        if (line_count % 10000 == 0) { // Exibe progresso a cada 10000 linhas
            printf("Processando linha %d...\n", line_count);
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Estatísticas:\n");
    printf("Erros 404: %d\n", count_404);
    printf("Total de bytes transferidos (status 200): %ld bytes\n", total_bytes);
    printf("Tempo total de execução: %.2f segundos\n", elapsed_time);

    fclose(file);
}

int main() {
    const char *log_file = "access_log_large.txt";
    analyze_log(log_file);
    return 0;
}

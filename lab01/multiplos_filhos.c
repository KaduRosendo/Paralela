# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/wait.h>

int main () {
    const int N = 4;
    pid_t pid;
    int i;

    printf("Processo Pai de PID: %d\n ", getppid());

    for (i = 0; i < N; i++) {
        pid = fork();

        if (pid < 0){
            fprintf(stderr, "Erro ao usar o fork\n");
            return 1;
        }else if (pid == 0) {
            printf("Filho %d | meu PID é %d, PID do meu pai é: %d\n", i+1, getpid(), getppid());

            exit(0);
        }
    }

    printf("\nPai esperando todos os filhos terminarem\n");
    for (i = 0; i < N; i++) {
        wait(NULL);
    }

    printf("Todos os filhos termianram\n");

    return 0;
}

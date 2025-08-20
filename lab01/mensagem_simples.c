 # include <stdio.h>
 # include <unistd.h>
 # include <sys/wait.h>

 int main () {
 pid_t pid = fork ();

 if ( pid < 0) { // Erro
    fprintf (stderr , " Fork falhou !\n");
    return 1;
 } else if (pid == 0) { // Processo Filho
    printf ("Eu sou o filho\n");
 } else { // Processo Pai
    printf ("Eu sou o pai\n");
    wait ( NULL ); // Pai espera o filho terminar
 }
 return 0;
}

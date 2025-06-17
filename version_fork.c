#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define GRUPOS 8
#define MAX_LINEA 256

char grupos[GRUPOS] = {'A','B','C','D','E','F','G','H'};

void proceso_hijo(int pipefd[2], char grupo) {
    close(pipefd[1]); // cerrar escritura del pipe

    char salida[30];
    sprintf(salida, "grupo_%c.txt", grupo);
    FILE *out = fopen(salida, "w");
    if (!out) {
        perror("Error abrir archivo de salida");
        exit(1);
    }

    char buffer[MAX_LINEA];
    ssize_t nbytes;

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    printf("[GRUPO %c] Inicio: %ld.%09ld\n", grupo, inicio.tv_sec, inicio.tv_nsec);

    // Leer desde pipe línea por línea
    FILE *pipe_stream = fdopen(pipefd[0], "r");
    if (!pipe_stream) {
        perror("fdopen");
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), pipe_stream)) {
        fputs(buffer, out);
    }

    fclose(out);
    fclose(pipe_stream);

    clock_gettime(CLOCK_MONOTONIC, &fin);
    printf("[GRUPO %c] Fin: %ld.%09ld\n", grupo, fin.tv_sec, fin.tv_nsec);
    printf("[GRUPO %c] Duracion: %.6f segundos\n", grupo,
        (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9);

    exit(0);
}

int main() {
    int pipes[GRUPOS][2];

    for (int i = 0; i < GRUPOS; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    for (int i = 0; i < GRUPOS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            // Proceso hijo
            for (int j = 0; j < GRUPOS; j++) {
                if (j != i) close(pipes[j][0]);
                close(pipes[j][1]);
            }
            proceso_hijo(pipes[i], grupos[i]);
        }
    }

    // Proceso padre
    // Cierra los extremos de lectura que no usa
    for (int i = 0; i < GRUPOS; i++) {
        close(pipes[i][0]);
    }

    FILE *archivo = fopen("~/Documents/Proyecto1B/pedidos.txt", "r");
    if (!archivo) {
        perror("Error abrir pedidos.txt");
        exit(1);
    }

    char linea[MAX_LINEA];
    while (fgets(linea, sizeof(linea), archivo)) {
        char grupo_linea = linea[0];
        for (int i = 0; i < GRUPOS; i++) {
            if (grupos[i] == grupo_linea) {
                write(pipes[i][1], linea, strlen(linea));
                break;
            }
        }
    }

    fclose(archivo);

    // Cerrar los extremos de escritura para indicar EOF a hijos
    for (int i = 0; i < GRUPOS; i++) {
        close(pipes[i][1]);
    }

    // Esperar hijos
    for (int i = 0; i < GRUPOS; i++) {
        wait(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);
    printf("\n[PROCESOS] Tiempo total: %.6f segundos\n",
        (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

    return 0;
}


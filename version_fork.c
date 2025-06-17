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

int contar_digitos(const char* str) {
    int suma = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            suma += (*str - '0');
        }
        str++;
    }
    return suma;
}

void proceso_hijo(int pipefd[2], int resfd[2], char grupo) {
    close(pipefd[1]);  // cerrar escritura de datos
    close(resfd[0]);   // cerrar lectura de resultados

    char archivo_salida[30];
    sprintf(archivo_salida, "grupo_%c.txt", grupo);
    FILE *out = fopen(archivo_salida, "w");
    if (!out) {
        perror("Error al abrir archivo de salida");
        exit(1);
    }

    FILE *pipe_stream = fdopen(pipefd[0], "r");
    if (!pipe_stream) {
        perror("fdopen");
        exit(1);
    }

    char buffer[MAX_LINEA];
    int suma_digitos = 0;

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    while (fgets(buffer, sizeof(buffer), pipe_stream)) {
        fputs(buffer, out);
        suma_digitos += contar_digitos(buffer);
    }

    // Escribir suma en archivo
    fprintf(out, "\nSuma de dígitos: %d\n", suma_digitos);

    fclose(out);
    fclose(pipe_stream);

    clock_gettime(CLOCK_MONOTONIC, &fin);

    // Enviar tiempos al padre para impresión ordenada
    double duracion = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;
    dprintf(resfd[1],
        "%c %ld.%09ld %ld.%09ld %.6f\n",
        grupo,
        inicio.tv_sec, inicio.tv_nsec,
        fin.tv_sec, fin.tv_nsec,
        duracion
    );

    close(resfd[1]);
    exit(0);
}

int main() {
    int pipes[GRUPOS][2];     // para enviar datos
    int result_pipes[GRUPOS][2]; // para recibir tiempos
    pid_t pids[GRUPOS];

    for (int i = 0; i < GRUPOS; i++) {
        if (pipe(pipes[i]) == -1 || pipe(result_pipes[i]) == -1) {
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
            // Cerrar otros pipes
            for (int j = 0; j < GRUPOS; j++) {
                if (j != i) {
                    close(pipes[j][0]); close(pipes[j][1]);
                    close(result_pipes[j][0]); close(result_pipes[j][1]);
                }
            }
            proceso_hijo(pipes[i], result_pipes[i], grupos[i]);
        } else {
            pids[i] = pid;
        }
    }

    // Padre: cerrar lectura de datos y escritura de resultados
    for (int i = 0; i < GRUPOS; i++) {
        close(pipes[i][0]);       // no leer de pipes de datos
        close(result_pipes[i][1]); // no escribir en pipes de resultados
    }

    // Leer archivo de pedidos
    FILE *archivo = fopen("/media/sf_CompartidaVM/pedidos.txt", "r");
    if (!archivo) {
        perror("Error abrir pedidos.txt");
        exit(1);
    }

    char linea[MAX_LINEA];
    while (fgets(linea, sizeof(linea), archivo)) {
        char grupo = linea[0];
        for (int i = 0; i < GRUPOS; i++) {
            if (grupos[i] == grupo) {
                write(pipes[i][1], linea, strlen(linea));
                break;
            }
        }
    }
    fclose(archivo);

    for (int i = 0; i < GRUPOS; i++) {
        close(pipes[i][1]); // cerrar escritura de datos
    }

    // Esperar hijos y leer resultados en orden A-H
    printf("\n--- RESULTADOS ORDENADOS (A-H) ---\n");
    for (int i = 0; i < GRUPOS; i++) {
        waitpid(pids[i], NULL, 0);

        char grupo;
        long ini_s, ini_ns, fin_s, fin_ns;
        double duracion;
        FILE* res_stream = fdopen(result_pipes[i][0], "r");
        fscanf(res_stream, "%c %ld.%ld %ld.%ld %lf", &grupo, &ini_s, &ini_ns, &fin_s, &fin_ns, &duracion);
        fclose(res_stream);

        printf("[GRUPO %c] Inicio: %ld.%09ld\n", grupo, ini_s, ini_ns);
        printf("[GRUPO %c] Fin: %ld.%09ld\n", grupo, fin_s, fin_ns);
        printf("[GRUPO %c] Duración: %.6f segundos\n\n", grupo, duracion);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);
    printf("[PROCESOS] Tiempo total: %.6f segundos\n",
        (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

    return 0;
}


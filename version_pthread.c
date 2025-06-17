#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define GRUPOS 8
#define MAX_LINEA 256
#define MAX_LINEAS_ARCHIVO 1000

char grupos[GRUPOS] = {'A','B','C','D','E','F','G','H'};
char *lineas[MAX_LINEAS_ARCHIVO];
int total_lineas = 0;

typedef struct {
    char grupo;
    struct timespec inicio;
    struct timespec fin;
    double duracion;
} DatosGrupo;

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

void* procesar_grupo(void* arg) {
    DatosGrupo* datos = (DatosGrupo*) arg;
    char grupo = datos->grupo;

    char archivo_salida[30];
    sprintf(archivo_salida, "grupo_%c.txt", grupo);
    FILE *out = fopen(archivo_salida, "w");
    if (!out) {
        perror("Error al abrir archivo de salida");
        pthread_exit(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &datos->inicio);

    int suma_total = 0;
    for (int i = 0; i < total_lineas; i++) {
        if (lineas[i][0] == grupo) {
            fputs(lineas[i], out);
            suma_total += contar_digitos(lineas[i]);
        }
    }

    fprintf(out, "\nSuma de dígitos: %d\n", suma_total);
    fclose(out);

    clock_gettime(CLOCK_MONOTONIC, &datos->fin);
    datos->duracion = (datos->fin.tv_sec - datos->inicio.tv_sec)
                    + (datos->fin.tv_nsec - datos->inicio.tv_nsec) / 1e9;

    pthread_exit(NULL);
}

int main() {
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    // Leer archivo en memoria
    FILE *archivo = fopen("/media/sf_CompartidaVM/pedidos.txt", "r");
    if (!archivo) {
        perror("Error al abrir pedidos.txt");
        exit(1);
    }

    char buffer[MAX_LINEA];
    while (fgets(buffer, sizeof(buffer), archivo) && total_lineas < MAX_LINEAS_ARCHIVO) {
        lineas[total_lineas] = strdup(buffer);
        if (!lineas[total_lineas]) {
            perror("strdup");
            exit(1);
        }
        total_lineas++;
    }
    fclose(archivo);

    // Crear hilos
    pthread_t hilos[GRUPOS];
    DatosGrupo datos[GRUPOS];

    for (int i = 0; i < GRUPOS; i++) {
        datos[i].grupo = grupos[i];
        pthread_create(&hilos[i], NULL, procesar_grupo, &datos[i]);
    }

    for (int i = 0; i < GRUPOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Liberar memoria
    for (int i = 0; i < total_lineas; i++) {
        free(lineas[i]);
    }

    // Imprimir resultados en orden A-H
    printf("\n--- RESULTADOS ORDENADOS (A-H) ---\n");
    for (int i = 0; i < GRUPOS; i++) {
        printf("[GRUPO %c] Inicio: %ld.%09ld\n", datos[i].grupo,
               datos[i].inicio.tv_sec, datos[i].inicio.tv_nsec);
        printf("[GRUPO %c] Fin: %ld.%09ld\n", datos[i].grupo,
               datos[i].fin.tv_sec, datos[i].fin.tv_nsec);
        printf("[GRUPO %c] Duración: %.6f segundos\n\n", datos[i].grupo,
               datos[i].duracion);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);
    printf("[HILOS] Tiempo total: %.6f segundos\n",
        (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

    return 0;
}


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

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char grupo;
} DatosGrupo;

typedef struct {
    char grupo;
    struct timespec inicio;
    struct timespec fin;
    double duracion;
} ResultadoGrupo;

ResultadoGrupo resultados[GRUPOS];

void* procesar_grupo(void* arg) {
    DatosGrupo* datos = (DatosGrupo*) arg;
    char grupo = datos->grupo;

    char salida[30];
    sprintf(salida, "grupo_%c.txt", grupo);
    FILE *out = fopen(salida, "w");
    if (!out) {
        perror("Error abrir archivo de salida");
        pthread_exit(NULL);
    }

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < total_lineas; i++) {
        if (lineas[i][0] == grupo) {
            fprintf(out, "%s", lineas[i]);
        }
    }

    fclose(out);
    clock_gettime(CLOCK_MONOTONIC, &fin);

    double duracion = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    int idx = grupo - 'A';
    resultados[idx].grupo = grupo;
    resultados[idx].inicio = inicio;
    resultados[idx].fin = fin;
    resultados[idx].duracion = duracion;

    pthread_exit(NULL);
}

int comparar_duracion(const void* a, const void* b) {
    ResultadoGrupo* r1 = (ResultadoGrupo*)a;
    ResultadoGrupo* r2 = (ResultadoGrupo*)b;
    if (r1->duracion < r2->duracion) return -1;
    else if (r1->duracion > r2->duracion) return 1;
    else return 0;
}

int main() {
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    // CAMBIA AQUÍ el nombre de usuario según tu sistema
    FILE *archivo = fopen("/home/anthony_gamboa_dev/Documents/sf_Proyecto_Ubuntu/pedidos.txt", "r");
    if (!archivo) {
        perror("Error abrir pedidos.txt");
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

    pthread_t hilos[GRUPOS];
    DatosGrupo datos[GRUPOS];

    for (int i = 0; i < GRUPOS; i++) {
        datos[i].grupo = grupos[i];
        pthread_create(&hilos[i], NULL, procesar_grupo, &datos[i]);
    }

    for (int i = 0; i < GRUPOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    for (int i = 0; i < total_lineas; i++) {
        free(lineas[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);

    qsort(resultados, GRUPOS, sizeof(ResultadoGrupo), comparar_duracion);

    printf("\n== RESULTADOS ORDENADOS POR DURACIÓN (menor a mayor) ==\n");
    for (int i = 0; i < GRUPOS; i++) {
        printf("[GRUPO %c] Inicio: %ld.%09ld\n", resultados[i].grupo,
               resultados[i].inicio.tv_sec, resultados[i].inicio.tv_nsec);
        printf("[GRUPO %c] Fin: %ld.%09ld\n", resultados[i].grupo,
               resultados[i].fin.tv_sec, resultados[i].fin.tv_nsec);
        printf("[GRUPO %c] Duracion: %.6f segundos\n\n", resultados[i].grupo,
               resultados[i].duracion);
    }

    printf("[HILOS] Tiempo total: %.6f segundos\n",
        (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

    return 0;
}


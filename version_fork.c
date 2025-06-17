#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define GRUPOS 8                  // Número de grupos (de 'A' a 'H')
#define MAX_LINEA 256             // Tamaño máximo por línea del archivo
#define MAX_LINEAS_ARCHIVO 1000   // Cantidad máxima de líneas que se pueden leer

// Letras que identifican a cada grupo
char grupos[GRUPOS] = {'A','B','C','D','E','F','G','H'};

// Arreglo para guardar las líneas leídas del archivo
char *lineas[MAX_LINEAS_ARCHIVO];
int total_lineas = 0; // Contador de líneas leídas

// Mutex para sincronizar impresión si se desea usar más adelante
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Estructura para pasar el grupo al hilo
typedef struct {
    char grupo;
} DatosGrupo;

// Estructura para guardar los tiempos de procesamiento de cada grupo
typedef struct {
    char grupo;
    struct timespec inicio;
    struct timespec fin;
    double duracion;
} ResultadoGrupo;

ResultadoGrupo resultados[GRUPOS];  // Resultados globales para todos los grupos

// Función que ejecutará cada hilo
void* procesar_grupo(void* arg) {
    DatosGrupo* datos = (DatosGrupo*) arg;
    char grupo = datos->grupo;

    // Construir nombre del archivo de salida para el grupo
    char salida[30];
    sprintf(salida, "grupo_%c.txt", grupo);
    FILE *out = fopen(salida, "w");
    if (!out) {
        perror("Error abrir archivo de salida");
        pthread_exit(NULL);
    }

    // Tiempo de inicio
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Procesar todas las líneas que pertenecen al grupo
    for (int i = 0; i < total_lineas; i++) {
        if (lineas[i][0] == grupo) {
            fprintf(out, "%s", lineas[i]);
        }
    }

    fclose(out);

    // Tiempo de fin
    clock_gettime(CLOCK_MONOTONIC, &fin);

    // Calcular duración en segundos (con nanosegundos incluidos)
    double duracion = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    // Guardar resultado en la posición correspondiente
    int idx = grupo - 'A';
    resultados[idx].grupo = grupo;
    resultados[idx].inicio = inicio;
    resultados[idx].fin = fin;
    resultados[idx].duracion = duracion;

    pthread_exit(NULL);
}

// Función de comparación para qsort (ordena por duración)
int comparar_duracion(const void* a, const void* b) {
    ResultadoGrupo* r1 = (ResultadoGrupo*)a;
    ResultadoGrupo* r2 = (ResultadoGrupo*)b;
    if (r1->duracion < r2->duracion) return -1;
    else if (r1->duracion > r2->duracion) return 1;
    else return 0;
}

int main() {
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio); // Tiempo total inicio

    // Cambia esta ruta según tu sistema
    FILE *archivo = fopen("/home/anthony_gamboa_dev/Documents/sf_Proyecto_Ubuntu/pedidos.txt", "r");
    if (!archivo) {
        perror("Error abrir pedidos.txt");
        exit(1);
    }

    // Leer línea por línea y almacenarlas en memoria
    char buffer[MAX_LINEA];
    while (fgets(buffer, sizeof(buffer), archivo) && total_lineas < MAX_LINEAS_ARCHIVO) {
        lineas[total_lineas] = strdup(buffer);  // Reservar y copiar la línea
        if (!lineas[total_lineas]) {
            perror("strdup");
            exit(1);
        }
        total_lineas++;
    }
    fclose(archivo);

    pthread_t hilos[GRUPOS];
    DatosGrupo datos[GRUPOS];

    // Crear un hilo por cada grupo
    for (int i = 0; i < GRUPOS; i++) {
        datos[i].grupo = grupos[i];
        pthread_create(&hilos[i], NULL, procesar_grupo, &datos[i]);
    }

    // Esperar que todos los hilos terminen
    for (int i = 0; i < GRUPOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Liberar memoria de las líneas leídas
    for (int i = 0; i < total_lineas; i++) {
        free(lineas[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin); // Tiempo total fin

    // Ordenar los resultados de los grupos por duración
    qsort(resultados, GRUPOS, sizeof(ResultadoGrupo), comparar_duracion);

    // Mostrar resultados por grupo
    printf("\n== RESULTADOS ORDENADOS POR DURACIÓN (menor a mayor) ==\n");
    for (int i = 0; i < GRUPOS; i++) {
        printf("[GRUPO %c] Inicio: %ld.%09ld\n", resultados[i].grupo,
               resultados[i].inicio.tv_sec, resultados[i].inicio.tv_nsec);
        printf("[GRUPO %c] Fin: %ld.%09ld\n", resultados[i].grupo,
               resultados[i].fin.tv_sec, resultados[i].fin.tv_nsec);
        printf("[GRUPO %c] Duracion: %.6f segundos\n\n", resultados[i].grupo,
               resultados[i].duracion);
    }

    // Mostrar tiempo total del programa principal
    printf("[HILOS] Tiempo total: %.6f segundos\n",
        (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

    return 0;
}

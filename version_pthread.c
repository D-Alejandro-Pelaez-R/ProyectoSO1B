#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define GRUPOS 8
#define MAX_LINEA 256
#define MAX_LINEAS_ARCHIVO 3000

char grupos[GRUPOS] = {'A','B','C','D','E','F','G','H'};
char *lineas[MAX_LINEAS_ARCHIVO];
int total_lineas = 0;

pthread_barrier_t barrier;  // Barrera para sincronizar inicio
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lineas_mutex = PTHREAD_MUTEX_INITIALIZER;

struct timespec tiempo_inicio_global;  // Tiempo de inicio común

typedef struct {
    char grupo;
    struct timespec inicio, fin;
    double duracion;
} DatosGrupo;

// Función para calcular la suma de los dígitos del ID
int suma_digitos(const char *id_str) {
    int suma = 0;
    for (int i = 0; id_str[i] != '\0'; i++) {
        if (id_str[i] >= '0' && id_str[i] <= '9') {
            suma += id_str[i] - '0';
        }
    }
    return suma;
}

// Convierte el ID a binario sin ceros iniciales
void convertir_a_binario(const char *id_str, char *binario_str, size_t tam) {
    int id = atoi(id_str);
    binario_str[0] = '\0';
    for (int i = sizeof(int) * 8 - 1; i >= 0; i--) {
        char bit = (id & (1 << i)) ? '1' : '0';
        strncat(binario_str, &bit, 1);
    }

    // Elimina ceros iniciales
    char *primer_uno = strchr(binario_str, '1');
    if (primer_uno) {
        memmove(binario_str, primer_uno, strlen(primer_uno) + 1);
    } else {
        strcpy(binario_str, "0");
    }
}

// Función para calcular el factorial de un número
unsigned long long factorial(int n) {
    if (n == 0 || n == 1) return 1;
    unsigned long long resultado = 1;
    for (int i = 2; i <= n; i++) {
        resultado *= i;
    }
    return resultado;
}

// Función para comparar tiempos (para qsort)
int comparar_duracion(const void *a, const void *b) {
    DatosGrupo *grupoA = (DatosGrupo *)a;
    DatosGrupo *grupoB = (DatosGrupo *)b;
    if (grupoA->duracion < grupoB->duracion) return -1;
    if (grupoA->duracion > grupoB->duracion) return 1;
    return 0;
}

// Función que ejecuta cada hilo
void* procesar_grupo(void* arg) {
    DatosGrupo* datos = (DatosGrupo*) arg;
    char grupo = datos->grupo;

    char nombre_archivo[30];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "grupo_%c.txt", grupo);
    FILE *archivo_salida = fopen(nombre_archivo, "w");
    if (!archivo_salida) {
        perror("Error al abrir archivo de salida");
        pthread_exit(NULL);
    }

    pthread_barrier_wait(&barrier);  // Sincronización global de inicio
    datos->inicio = tiempo_inicio_global;

    fprintf(archivo_salida, "Grupo %c\n", grupo);

    for (int i = 0; i < total_lineas; i++) {
        pthread_mutex_lock(&lineas_mutex);  // proteger acceso a lineas[i]
        int es_grupo = (lineas[i][0] == grupo);
        char *linea_copia = strdup(lineas[i]);
        pthread_mutex_unlock(&lineas_mutex);

        if (!es_grupo || !linea_copia) {
            if (linea_copia) free(linea_copia);
            continue;
        }

        char *token = strtok(linea_copia, ","); // grupo
        token = strtok(NULL, ",");              // id
        char *id_str = token ? token : "";
        token = strtok(NULL, "\n");             // nombre libro
        char *nombre_libro = token ? token : "";

        int suma = suma_digitos(id_str);
        unsigned long long fact = factorial(suma);

        char binario[50];
        convertir_a_binario(id_str, binario, sizeof(binario));

        int id_int = atoi(id_str);
        char hex_str[20];
        snprintf(hex_str, sizeof(hex_str), "%X", id_int);

        fprintf(archivo_salida, "%s, ID: %s, Suma ID: %d, ID EN BINARIO: %s, ID EN HEXADECIMAL: %s, FACTORIAL SUMA: %llu\n",
                nombre_libro, id_str, suma, binario, hex_str, fact);

        free(linea_copia);
    }

    fclose(archivo_salida);

    clock_gettime(CLOCK_MONOTONIC, &datos->fin);

    datos->duracion = (datos->fin.tv_sec - datos->inicio.tv_sec) +
                      (datos->fin.tv_nsec - datos->inicio.tv_nsec) / 1e9;

    pthread_exit(NULL);
}

int main() {
    struct timespec tiempo_fin;
    clock_gettime(CLOCK_MONOTONIC, &tiempo_inicio_global);  // Tiempo global inicial

    // Leer archivo
    FILE *archivo = fopen("/media/sf_Proyecto_Ubuntu/pedidos.txt", "r");
    if (!archivo) {
        perror("Error al abrir pedidos.txt");
        return 1;
    }

    char buffer[MAX_LINEA];
    while (fgets(buffer, sizeof(buffer), archivo) && total_lineas < MAX_LINEAS_ARCHIVO) {
        lineas[total_lineas] = strdup(buffer);
        if (!lineas[total_lineas]) {
            perror("Error al duplicar línea");
            fclose(archivo);
            return 1;
        }
        total_lineas++;
    }
    fclose(archivo);

    pthread_t hilos[GRUPOS];
    DatosGrupo datos[GRUPOS];

    pthread_barrier_init(&barrier, NULL, GRUPOS);

    // Crear hilos
    for (int i = 0; i < GRUPOS; i++) {
        datos[i].grupo = grupos[i];
        if (pthread_create(&hilos[i], NULL, procesar_grupo, &datos[i]) != 0) {
            fprintf(stderr, "Error creando hilo para grupo %c\n", grupos[i]);
            return 1;
        }
    }

    // Esperar hilos
    for (int i = 0; i < GRUPOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    // Liberar memoria
    for (int i = 0; i < total_lineas; i++) {
        free(lineas[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    double tiempo_total = (tiempo_fin.tv_sec - tiempo_inicio_global.tv_sec) +
                          (tiempo_fin.tv_nsec - tiempo_inicio_global.tv_nsec) / 1e9;

    // Ordenar por menor duración
    qsort(datos, GRUPOS, sizeof(DatosGrupo), comparar_duracion);

    // Imprimir resultados sincronizados
    pthread_mutex_lock(&print_mutex);
    printf("=== RESULTADOS POR GRUPO (ORDENADOS POR DURACION) ===\n\n");
    for (int i = 0; i < GRUPOS; i++) {
        printf("[GRUPO %c] INICIO   : %ld.%09ld\n", datos[i].grupo, datos[i].inicio.tv_sec, datos[i].inicio.tv_nsec);
        printf("[GRUPO %c] FIN      : %ld.%09ld\n", datos[i].grupo, datos[i].fin.tv_sec, datos[i].fin.tv_nsec);
        printf("[GRUPO %c] DURACION : %.6f segundos\n\n", datos[i].grupo, datos[i].duracion);
    }

    printf("===========================================\n");
    printf("[HILOS] TIEMPO TOTAL: %.6f segundos\n", tiempo_total);
    printf("===========================================\n");
    pthread_mutex_unlock(&print_mutex);

    return 0;
}


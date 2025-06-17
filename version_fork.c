// === INCLUSIÓN DE BIBLIOTECAS ===
#include <stdio.h>    // Para entrada/salida estándar
#include <stdlib.h>   // Para funciones como malloc, free, atoi
#include <string.h>   // Para manipulación de strings
#include <unistd.h>   // Para fork(), pipe(), close()
#include <sys/wait.h> // Para waitpid()
#include <time.h>     // Para medición de tiempos
#include <fcntl.h>    // Para operaciones con archivos
#include <semaphore.h>// Para semáforos
#include <sys/mman.h> // Para memoria compartida (mmap)

// === CONSTANTES DEL PROGRAMA ===
#define GRUPOS 8                 // Número de grupos a procesar (A-H)
#define MAX_LINEA 256            // Longitud máxima de una línea de texto
#define MAX_LINEAS_ARCHIVO 3000  // Máximo de líneas a leer del archivo

// === VARIABLES GLOBALES ===
char grupos[GRUPOS] = {'A','B','C','D','E','F','G','H'}; // Letras de los grupos
char *lineas[MAX_LINEAS_ARCHIVO]; // Almacena las líneas del archivo
int total_lineas = 0;             // Contador de líneas leídas

// Estructura para guardar el tiempo de inicio global del programa
struct timespec tiempo_inicio_global;

// Estructura para almacenar datos de tiempo de cada grupo
typedef struct {
    char grupo;              // Letra del grupo (A-H)
    struct timespec inicio;  // Tiempo de inicio del procesamiento
    struct timespec fin;     // Tiempo de finalización
    double duracion;         // Duración total en segundos
} DatosGrupo;

// === FUNCIONES UTILITARIAS ===

// Calcula la suma de los dígitos de un string numérico
int suma_digitos(const char *id_str) {
    int suma = 0;
    for (int i = 0; id_str[i] != '\0'; i++) {
        if (id_str[i] >= '0' && id_str[i] <= '9') {
            suma += id_str[i] - '0'; // Convierte char a int y suma
        }
    }
    return suma;
}

// Convierte un string numérico a su representación binaria
void convertir_a_binario(const char *id_str, char *binario_str, size_t tam) {
    int id = atoi(id_str); // Convierte string a entero
    binario_str[0] = '\0'; // Inicializa el string de resultado

    // Recorre cada bit del entero (de más significativo a menos)
    for (int i = sizeof(int) * 8 - 1; i >= 0; i--) {
        char bit = (id & (1 << i)) ? '1' : '0'; // Obtiene el bit actual
        strncat(binario_str, &bit, 1);          // Añade el bit al string
    }

    // Elimina ceros a la izquierda
    char *primer_uno = strchr(binario_str, '1');
    if (primer_uno) {
        memmove(binario_str, primer_uno, strlen(primer_uno) + 1);
    } else {
        strcpy(binario_str, "0"); // Si todos son ceros, devuelve "0"
    }
}

// Calcula el factorial de un número (iterativo)
unsigned long long factorial(int n) {
    if (n == 0 || n == 1) return 1;
    unsigned long long resultado = 1;
    for (int i = 2; i <= n; i++) {
        resultado *= i;
    }
    return resultado;
}

// Función de comparación para qsort (ordena por duración)
int comparar_duracion(const void *a, const void *b) {
    DatosGrupo *grupoA = (DatosGrupo *)a;
    DatosGrupo *grupoB = (DatosGrupo *)b;
    // Devuelve -1, 0 o 1 según la comparación
    return (grupoA->duracion > grupoB->duracion) - (grupoA->duracion < grupoB->duracion);
}

// Imprime un tiempo en formato legible (segundos.nanosegundos)
void imprimir_tiempo_legible(const char *label, struct timespec ts) {
    printf("%s: %ld.%09ld segundos desde inicio\n", label, ts.tv_sec, ts.tv_nsec);
}

// === FUNCIÓN PARA PROCESAR CADA GRUPO ===
void procesar_grupo(char grupo, char **lineas, int total_lineas, int pipe_fd[2], sem_t *sem) {
    close(pipe_fd[0]); // Cerramos el extremo de lectura del pipe (solo escribiremos)

    // Marcamos tiempo de inicio del procesamiento
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Entramos a sección crítica (protegida por semáforo para acceso a archivos)
    sem_wait(sem);

    // Creamos archivo de salida para este grupo
    char nombre_archivo[30];
    snprintf(nombre_archivo, sizeof(nombre_archivo), "grupo_%c.txt", grupo);
    FILE *archivo_salida = fopen(nombre_archivo, "w");
    if (!archivo_salida) {
        perror("Error al abrir archivo de salida");
        sem_post(sem); // Liberamos semáforo antes de salir
        exit(EXIT_FAILURE);
    }

    // Escribimos cabecera del archivo
    fprintf(archivo_salida, "Grupo %c\n", grupo);

    // Procesamos cada línea del archivo original
    for (int i = 0; i < total_lineas; i++) {
        // Saltamos líneas que no correspondan a este grupo
        if (lineas[i][0] != grupo) continue;

        // Duplicamos la línea para no modificar la original
        char *linea_copia = strdup(lineas[i]);
        if (!linea_copia) {
            perror("Error duplicando línea");
            fclose(archivo_salida);
            sem_post(sem);
            exit(EXIT_FAILURE);
        }

        // Parseamos la línea (formato: grupo,id,libro)
        char *token = strtok(linea_copia, ","); // Obtenemos grupo (ya sabemos que coincide)
        token = strtok(NULL, ",");               // Obtenemos ID
        char *id_str = token ? token : "";      // Guardamos ID (o string vacío si es NULL)
        token = strtok(NULL, "\n");             // Obtenemos nombre del libro
        char *nombre_libro = token ? token : "";

        // Realizamos cálculos con el ID
        int suma = suma_digitos(id_str);               // Suma de dígitos
        unsigned long long fact = factorial(suma);      // Factorial de la suma
        char binario[50];
        convertir_a_binario(id_str, binario, sizeof(binario)); // Conversión a binario
        int id_int = atoi(id_str);
        char hex_str[20];
        snprintf(hex_str, sizeof(hex_str), "%X", id_int); // Conversión a hexadecimal

        // Escribimos resultados en el archivo
        fprintf(archivo_salida, "%s, ID: %s, Suma ID: %d, ID BIN: %s, HEX: %s, FACT: %llu\n",
                nombre_libro, id_str, suma, binario, hex_str, fact);

        free(linea_copia); // Liberamos memoria de la copia
    }

    fclose(archivo_salida);
    sem_post(sem); // Liberamos el semáforo (salimos de sección crítica)

    // Marcamos tiempo de finalización y calculamos duración
    clock_gettime(CLOCK_MONOTONIC, &fin);
    double duracion = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    // Preparamos datos para enviar al proceso padre
    DatosGrupo datos;
    datos.grupo = grupo;
    datos.inicio = inicio;
    datos.fin = fin;
    datos.duracion = duracion;

    // Enviamos datos por el pipe y cerramos descriptor
    write(pipe_fd[1], &datos, sizeof(DatosGrupo));
    close(pipe_fd[1]);

    exit(EXIT_SUCCESS); // Finalizamos el proceso hijo
}

// === FUNCIÓN PRINCIPAL ===
int main() {
    struct timespec tiempo_fin;
    // Marcamos tiempo de inicio global del programa
    clock_gettime(CLOCK_MONOTONIC, &tiempo_inicio_global);

    // === LECTURA DEL ARCHIVO ===
    FILE *archivo = fopen("/media/sf_CompartidaVM/pedidos.txt", "r");
    if (!archivo) {
        perror("Error abriendo pedidos.txt");
        return 1;
    }

    // Leemos todas las líneas del archivo
    char buffer[MAX_LINEA];
    while (fgets(buffer, sizeof(buffer), archivo) && total_lineas < MAX_LINEAS_ARCHIVO) {
        lineas[total_lineas] = strdup(buffer); // Duplicamos cada línea
        if (!lineas[total_lineas]) {
            perror("Error duplicando línea");
            fclose(archivo);
            return 1;
        }
        total_lineas++;
    }
    fclose(archivo);

    // === CONFIGURACIÓN DE SEMÁFORO ===
    // Creamos semáforo en memoria compartida (para sincronizar acceso a archivos)
    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem == MAP_FAILED) {
        perror("mmap error");
        exit(EXIT_FAILURE);
    }
    sem_init(sem, 1, 1); // Inicializamos semáforo (compartido entre procesos, valor inicial 1)

    // === CREACIÓN DE PROCESOS HIJOS ===
    pid_t pids[GRUPOS];      // Almacena IDs de los procesos hijos
    DatosGrupo datos[GRUPOS]; // Almacena datos de tiempo de cada grupo
    int pipes[GRUPOS][2];     // Pipes para comunicación con cada hijo

    for (int i = 0; i < GRUPOS; i++) {
        // Creamos pipe para comunicación con este hijo
        if (pipe(pipes[i]) == -1) {
            perror("Error creando pipe");
            exit(EXIT_FAILURE);
        }

        // Creamos proceso hijo
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Código del hijo
            // Cerramos pipes que no usaremos
            for (int j = 0; j < GRUPOS; j++) {
                if (j != i) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }
            // Procesamos el grupo correspondiente
            procesar_grupo(grupos[i], lineas, total_lineas, pipes[i], sem);
        } else { // Código del padre
            pids[i] = pid; // Guardamos ID del hijo
            close(pipes[i][1]); // Cerramos extremo de escritura del pipe (solo leeremos)
        }
    }

    // === RECOLECCIÓN DE RESULTADOS ===
    // Leemos datos de tiempo de cada hijo a través de los pipes
    for (int i = 0; i < GRUPOS; i++) {
        ssize_t leido = read(pipes[i][0], &datos[i], sizeof(DatosGrupo));
        if (leido != sizeof(DatosGrupo)) {
            fprintf(stderr, "Error leyendo pipe del grupo %c\n", grupos[i]);
            datos[i].grupo = grupos[i];
            datos[i].duracion = 0;
        }
        close(pipes[i][0]); // Cerramos el pipe después de leer
    }

    // Esperamos a que todos los hijos terminen
    for (int i = 0; i < GRUPOS; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // === CÁLCULO DE TIEMPOS ===
    // Marcamos tiempo final y calculamos duración total
    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    double tiempo_total = (tiempo_fin.tv_sec - tiempo_inicio_global.tv_sec) +
                         (tiempo_fin.tv_nsec - tiempo_inicio_global.tv_nsec) / 1e9;

    // Ordenamos los grupos por duración (de menor a mayor)
    qsort(datos, GRUPOS, sizeof(DatosGrupo), comparar_duracion);

    // === IMPRESIÓN DE RESULTADOS ===
    printf("\n=== RESULTADOS POR GRUPO (ORDENADOS POR DURACION) ===\n\n");
    for (int i = 0; i < GRUPOS; i++) {
        printf("Grupo %c\n", datos[i].grupo);
        imprimir_tiempo_legible("  Inicio", datos[i].inicio);
        imprimir_tiempo_legible("  Fin   ", datos[i].fin);
        printf("  Duracion: %.6f segundos\n\n", datos[i].duracion);
    }

    printf("===========================================\n");
    printf("TIEMPO TOTAL DE EJECUCION: %.6f segundos\n", tiempo_total);
    printf("===========================================\n");

    // === LIMPIEZA ===
    // Liberamos memoria de las líneas leídas
    for (int i = 0; i < total_lineas; i++) {
        free(lineas[i]);
    }

    // Destruimos el semáforo
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));

    return 0;
}
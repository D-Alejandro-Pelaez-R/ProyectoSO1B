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

// Solo para sincronizar impresi�n en pantalla (opcional)
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
char grupo;
} DatosGrupo;

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
pthread_mutex_lock(&print_mutex);
printf("[GRUPO %c] Inicio: %ld.%09ld\n", grupo, inicio.tv_sec, inicio.tv_nsec);
pthread_mutex_unlock(&print_mutex);

for (int i = 0; i < total_lineas; i++) {
if (lineas[i][0] == grupo) {
fprintf(out, "%s", lineas[i]);
}
}

fclose(out);

clock_gettime(CLOCK_MONOTONIC, &fin);
pthread_mutex_lock(&print_mutex);
printf("[GRUPO %c] Fin: %ld.%09ld\n", grupo, fin.tv_sec, fin.tv_nsec);
printf("[GRUPO %c] Duracion: %.6f segundos\n", grupo,
(fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9);
pthread_mutex_unlock(&print_mutex);

pthread_exit(NULL);
}

int main() {
struct timespec t_inicio, t_fin;
clock_gettime(CLOCK_MONOTONIC, &t_inicio);

// Cargar todo el archivo en memoria
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

// Liberar memoria
for (int i = 0; i < total_lineas; i++) {
free(lineas[i]);
}

clock_gettime(CLOCK_MONOTONIC, &t_fin);
printf("\n[HILOS] Tiempo total: %.6f segundos\n",
(t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9);

return 0;
}

# Proyecto 1B - Sistemas Operativos
## Ejecución de múltiples tareas con procesos y con hilos

**Integrantes:**
- Jean Pierre Espinosa
- Anthony Gamboa
- Alejandro Peláez
- Christian Yanchatipan

### Problema



### Ejecución

**NOTA:** Este proyecto se realizó en lenguaje C y en el terminal del sistema operativo Linux, por ello, se recomienda realizar la ejecución en la terminal de linux, y se debe asegurar que tiene instalado el lenguaje C en su maquina.

1. Clone el repositorio en el directorio que desee. Para facilitar la ejecución, se recomienda que descargue unicamente los archivos '.c' de la rama main del repositorio, más no clonar todo el repositorio de este proyecto.

2. Antes de ejecutar el codigo, asegurese de cambiar en el codigo (version_fork.c y version_pthread.c) las líneas de codigo en dónde se especifique el directorio del proyecto por el directorio donde descargo/copio los archivos del paso 1 (Observar la siguiente imagen). 
![Screenshot from 2025-06-16 22-09-04](https://github.com/user-attachments/assets/ccebe278-6b2d-40ff-afa3-52c463359bf6)

Para ello en su terminal de linux, ejecute el siguiente comando $ vi version_pthread.c . Observará que se abre un editor de texto, busque la línea de codigo que se muestra en la imagen anterior, luego edite el directorio del codigo por el directorio en el que se encuentra su proyecto. Salga del editor de texto guardando los cambios que realizó. Realize lo mismo para el archivo version_pthread.c

3. Ejecute el codigo en el terminal de linux. Para ello ejecute el siguiente comando $ gcc -pthread version_pthread.c -o version_pthread , luego ejecute $ ./version_pthread . Observer los resultados por pantalla de cada uno de los ejecutables. Repetir el proceso ahora para el ejecutable version_fork.c, con la diferencia que se ejecuta el siguiente comando $ gcc -version_fork.c -o version_fork

4. Además, podrá observar que en su repositorio se habrán creado archivos adicionales correspondientes a la ejecución de los programas.
![Screenshot from 2025-06-16 23-30-09](https://github.com/user-attachments/assets/17413e9c-94e0-4601-abd2-b8707184b6a8)

### Resultados

**version_pthread:** Al ejecutar el programa podrá observar en la salida de pantalla de su terminal lo siguiente: 
![Screenshot from 2025-06-16 23-48-41](https://github.com/user-attachments/assets/9f2fbb95-d261-4b08-ada3-2f0ea08059a9)


**version_fork:** Al ejecutar el programa podrá observar por pantalla lo siguiente:



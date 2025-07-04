# Proyecto 1B - Sistemas Operativos
## Ejecución de múltiples tareas con procesos y con hilos

**Integrantes:**
- Jean Pierre Espinosa
- Anthony Gamboa
- Alejandro Peláez
- Christian Yanchatipan

### Problema

Una librería necesita crear un programa que clasifique los pedidos encontrados en un archivo "pedidos.txt" por grupos, por IDs y nombres. Además que debe subdividir los pedidos según el grupo, también debe sumar las IDs de cada pedido y sacar su factorial, y transformar las IDs en binario y hexadecimal. Le han ofertado dos alternativas de programas, uno usando hilos y otro con procesos. Determinar que programa es el más eficiente.

### Ejecución

**NOTA:** Este proyecto se realizó en lenguaje C y en el terminal del sistema operativo Linux, por ello, se recomienda realizar la ejecución en la terminal de linux, y se debe asegurar que tiene instalado el lenguaje C en su maquina.

1. Clone el repositorio en el directorio que desee. Para facilitar la ejecución, se recomienda que descargue unicamente los archivos '.c' de la rama main del repositorio, más no clonar todo el repositorio de este proyecto.

2. Antes de ejecutar el código, asegurese de cambiar en el código (version_fork.c y version_pthread.c) las líneas de codigo en dónde se especifique el directorio del proyecto por el directorio donde descargo/copio los archivos del paso 1 (Observar la siguiente imagen). 
![Screenshot from 2025-06-16 22-09-04](https://github.com/user-attachments/assets/ccebe278-6b2d-40ff-afa3-52c463359bf6)

Para ello en su terminal de linux, ejecute el siguiente comando $ vi version_pthread.c . Observará que se abre un editor de texto, busque la línea de código que se muestra en la imagen anterior, luego edite el directorio del código por el directorio en el que se encuentra su proyecto. Salga del editor de texto guardando los cambios que realizó. Realize lo mismo para el archivo version_pthread.c

3. Ejecute el codigo en el terminal de linux. Para ello ejecute el siguiente comando $ gcc -pthread version_pthread.c -o version_pthread , luego ejecute $ ./version_pthread . Observer los resultados por pantalla de cada uno de los ejecutables. Repetir el proceso ahora para el ejecutable version_fork.c, con la diferencia que se ejecuta el siguiente comando $ gcc -version_fork.c -o version_fork

4. Además, podrá observar que en su repositorio se habrán creado archivos adicionales correspondientes a la ejecución de los programas.
![Screenshot from 2025-06-16 23-30-09](https://github.com/user-attachments/assets/17413e9c-94e0-4601-abd2-b8707184b6a8)

### Resultados

**version_pthread:** Al ejecutar el programa podrá observar en la salida de pantalla de su terminal lo siguiente: 
![pthread1](https://github.com/user-attachments/assets/ffcbf461-b773-472f-b634-dd3d8db5307a)
![pthread2](https://github.com/user-attachments/assets/2b80995e-ea8a-4baf-94e5-751d97a9edc4)


**version_fork:** Al ejecutar el programa podrá observar por pantalla lo siguiente:
![fork1](https://github.com/user-attachments/assets/2b62a4d7-addb-457d-883e-c23dd67accaa)
![fork2](https://github.com/user-attachments/assets/2aa72697-13c4-45d8-a6aa-22affbe2d1ba)

### Conclusiones

- La ejecución de las tareas resultó más eficiente en con hilos que con procesos.
- Se evindeció algunas diferencias entre los programas, como las implementación de sincronización en cada uno, es así, que el progama implemetnado con hilos usó mutex y barreras, mientras que en que el programa implementado con procesos usó semaforos.
- Se debe diseñar programas basados en hilos al necesitar la ejecución de multiples tareas o tareas mas complejas
- Se idintificó una condición de carrera en la impresión de datos por pantalla en el programa basado en hilos, se solucionó agregando mutex para ejecución de este apartado. Por otro lado, se identificó la poca eficiencia de la ejecución de tareas en el programa basado en proceso, esto debido a que se implemento una sincronización separando por tiempos definidos cada proceso; se solucionó implementando un semaforo para los procesos.

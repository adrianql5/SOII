// Miembros equipo: Adrián Quiroga Linares, Lucía Pérez González

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define N 8    // Tamaño de la cola

/* Estructura compartida entre el productor y el consumidor */
struct Buffer {
    char cola[N];  // Cola de caracteres
    int inicio;    // Índice del principio de la cola
    int fin;       // Índice del fin de la cola
    int numElem;   // Contador del número de elementos
} buffer; // Variable global

/* Estructura que guarda los valores introducidos por el usuario */
typedef struct {
    char fichero[100]; // Nombre del fichero a abrir/crear
    int T;             // Valor pasado a los sleep()
    int numHilo;       // Índice del hilo
} Datos;

// Mutex y barrera como variables globales
pthread_mutex_t mutex;
pthread_barrier_t barrera;

// Variables globales para el control de hilos
int numProd, contProd;

/* Función de lectura de un caracter del fichero indicado para generar un item */
char generarItem(FILE *fp) {
    char item = getc(fp);
    if (item == EOF) return '*';
    if (item == '*') return '\0';
    return item;
}

/* Función para insertar un elemento en la cola */
void insertarItem(char item, Datos *datos) {
    buffer.cola[buffer.fin] = item;
    buffer.numElem++;
    
    printf(" Prod %d: Introdujo %c en la posición %d |\n", datos->numHilo, item, buffer.fin);
    
    buffer.fin = (buffer.fin + 1) % N;
}

/* Función para retirar un elemento de la cola y escribirlo en un fichero */
char retirarItem(FILE *fp, Datos *datos) {
    if (buffer.numElem == 0) return '\0';

    char item = buffer.cola[buffer.inicio];
    buffer.cola[buffer.inicio] = '\0';
    buffer.numElem--;
    
    printf("\t\t\t\t      | Cons %d: Retiró %c de la posición %d\n", datos->numHilo, item, buffer.inicio);
    
    if (item != '*') {
        putc(item, fp);
    }

    buffer.inicio = (buffer.inicio + 1) % N;
    return item;
}

/* Hilo productor */
void *productor(void *arg) {
    Datos *datosProd = (Datos*)arg;

    // Se abre en modo lectura el fichero de items del hilo productor
    FILE *fp;
    if ((fp = fopen(datosProd->fichero, "r")) == NULL) {
        perror("Error al abrir archivo para lectura");
        pthread_exit(NULL);
    }

    //Sincronizamos a los productores
    pthread_barrier_wait(&barrera);

    // Proceso de producción
    /* Solamente se leerán caracteres alfanuméricos del fichero, por lo que cuando se llegue
     * al final de este, para notificar el fin de la producción de este hilo, se produce 
     * un asterico como item. Esto implica que el hilo de producción continuará 
     * hasta que se produzca un asterisco
     */
    char item = '\0';
    while (item != '*') {
        // Simula tiempo de producción
        sleep(rand() % (datosProd->T + 1));

        // Se lee un caracter del fichero de lectura y se devuelve un item
        item = generarItem(fp);
        // Se comprueba que el item devuelto sea un caracter alfanumérico o un asterico de finalización
        if ((item >= 48 && item <= 57) || (item >= 65 && item <= 90) || (item >= 97 && item <= 122) || item == '*') {
            // Bloquea el acceso a la región crítica
            pthread_mutex_lock(&mutex);
            while (buffer.numElem == N) {
                // Mientras el buffer compartido está lleno, cede su cpu automaticamente para que trabaje otro hilo,
                // liberando y bloqueando el mutex antes y después de ello
                pthread_mutex_unlock(&mutex);
                sched_yield(); 
                pthread_mutex_lock(&mutex);
            }
            // Se inserta el item en el buffer compartido
            insertarItem(item, datosProd);

            // Libera el acceso a región crítica
            pthread_mutex_unlock(&mutex);
        }
    }

    // Se cierra el fichero de lectura, se libera la memoria de datosProd y se finaliza el hilo
    fclose(fp);
    free(datosProd);
    pthread_exit(NULL);
}

/* Hilo consumidor */
void *consumidor(void *arg) {
    Datos *datosCons = (Datos*)arg;
    
    // Se crea en modo escritura el fichero del hilo consumidor
    FILE *fp;
    if ((fp = fopen(datosCons->fichero, "w")) == NULL) {
        perror("Error al crear archivo de salida");
        pthread_exit(NULL);
    }

    // Sincronizamos a los consumidores
    pthread_barrier_wait(&barrera);

    // Proceso de consumo
    /* Al recibir un asterisco por parte de un productor, se incrementa la variable contProd
     * Mientras esta variable no iguale el número de productores creados no se finaliza con el consumo 
     */
    while (1) {
        // Simula tiempo de consumo
        sleep(rand() % (datosCons->T + 1));

        // Bloquea el acceso a la región crítica
        pthread_mutex_lock(&mutex);
        while (buffer.numElem == 0) {
            // Si el número de productores finalizador (número de asteriscos recibidos) iguala el
            // número de hilos productores creados (número de asteriscos a recibir), entonces se finaliza la ejecución
            if (contProd == numProd) {
                // Libera el acceso a región crítica
                pthread_mutex_unlock(&mutex);

                // Se cierra el fichero de escritura, se libera la memoria de datosProd y se finaliza el hilo
                fclose(fp);
                free(datosCons);
                pthread_exit(NULL);
            }
            // Mientras el buffer compartido está vacío, cede su cpu automaticamente para que trabaje otro hilo
            // liberando y bloqueando el mutex antes y después de ello
            pthread_mutex_unlock(&mutex);
            sched_yield(); 
            pthread_mutex_lock(&mutex);
        }

        // Se extrae el primer item del buffer y se escribe en el fichero propio
        char item = retirarItem(fp, datosCons);

        // Si el item es un asterisco, se incrementa el contador contProd
        if (item == '*') contProd++;

        // Libera el acceso a región crítica
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// Main
int main(int argc, char **argv) {
    // Paso 1: Se comprueba que se introduce el número de argumentos adecuado y de formato adecuado por línea de comandos
    int numCons;
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <num_prod> <num_cons>\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if((numProd=atoi(argv[1]))==0){
            if(strcmp(argv[1], "0")==0){
                perror("El número de productores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_prod> <num_cons> ");
            }else{
                perror("El primer parámetro no es un número entero. Formato aceptado: ./exe <num_prod> <num_cons> ");
            }
            exit(EXIT_FAILURE);
        }
        if((numCons=atoi(argv[2]))==0){
            if(strcmp(argv[2], "0")==0){
                perror("El número de consumidores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_prod> <num_cons> ");
            }else{
                perror("El segundo parámetro no es un número entero. Formato aceptado: ./exe <num_prod> <num_cons> ");
            }
            exit(EXIT_FAILURE);
        }
        if(numProd<0 || numCons<0){
            fprintf(stderr, "Los parámetros deben ser enteros positivos.\n");
            (EXIT_FAILURE);
        }
    }

    // Paso 2: Se inicializan variables necesarias, la barrera para poder emplearla entre hilos distintos
    // y los mutexes que se emplearán entre hilos y entre productor y consumidor
    contProd = 0;
    // Inicializar la cola vacía
    buffer.inicio = 0;
    buffer.fin = 0;
    buffer.numElem = 0;
    // Inicializar mutexes y barrera
    pthread_mutex_init(&mutex, NULL);
    pthread_barrier_init(&barrera, NULL, numProd + numCons);

    // Paso 3: Se crear hilos de productor y consumidor
    pthread_t t_prod[numProd], t_cons[numCons];
    for (int i = 0; i < numProd; i++) {
        // Se crea e inicializa la estructura de datos para el productor datosProd
        // y se pide al usuario el nombre del fichero de lectura y el tiempo para el sleep()
        Datos *datosProd = malloc(sizeof(Datos));
        datosProd->numHilo = i + 1;

        printf("Productor %d) Fichero a emplear: ", i + 1);
        scanf(" %s", datosProd->fichero);
        printf("\t     Tiempo a dormir el hilo: ");
        scanf(" %d", &datosProd->T);

        // Se crea el hilo productor pasándole la estructura de datos como parámetro
        pthread_create(&t_prod[i], NULL, productor, datosProd);
    }

    for (int i = 0; i < numCons; i++) {
        // Se crea e inicializa la estructura de datos para el consumidor datosCons
        // y se pide al usuario el tiempo para el sleep()
        Datos *datosCons = malloc(sizeof(Datos));
        datosCons->numHilo = i + 1;

        printf("Consumidor %d) Tiempo a dormir el hilo: ", i + 1);
        scanf(" %d", &datosCons->T);
        // Se declara el nombre del fichero de escritura propio del hilo consumidor
        sprintf(datosCons->fichero, "fileCons_%d.txt", datosCons->numHilo);

        // Se crea el hilo consumidor pasándole la estructura de datos como parámetro
        pthread_create(&t_cons[i], NULL, consumidor, datosCons);
    }

    // Se formatea la impresión por pantalla 
    printf("\n\t\tPRODUCTORES\t      |\t\tCONSUMIDORES\n");
    printf("_________________________________________________________________________\n");

    // Paso 4: Se epera a que los hilos terminen
    for (int i = 0; i < numProd; i++) {
        pthread_join(t_prod[i], NULL);
    }

    for (int i = 0; i < numCons; i++) {
        pthread_join(t_cons[i], NULL);
    }

    // Paso 5: Se destruiyen los mutexes y barrera
    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrera);

    return 0;
}


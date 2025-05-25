// Miembros equipo: Adrián Quiroga Linares, Lucía Pérez González

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <wait.h>
#include <string.h>

#define N 8    // Tamaño de la cola

/* Estructuras compartida entre el productor y el consumidor */
typedef struct {
    char cola[N];  // Cola de caracteres
    int inicio;    // Índice del principio de la cola
    int fin;       // Índice del fin de la cola
    int numElem;   // Contador del número de elementos
} Buffer; 

typedef struct{
    // Mutexes
    pthread_mutex_t mutex;
    pthread_cond_t condc, condp;
    // Buffer
    Buffer buffer;
    // Variables
    int numProd;
} CompartidoExt;

/* Estructura compartida solamente entre consumidores */
typedef struct{
        // Barrera
        pthread_barrier_t barrera;
} CompartidoInt;

/* Variables globales */
CompartidoExt *compartidoExt; 
CompartidoInt *compartidoInt;

/* Función de lectura de un caracter del fichero indicado */
char generarItem(FILE *fp) {
    char item= getc(fp);
    if(item == EOF) return '*';
    if (item == '*') return '\0';
    return item;
}

/* Función para insertar un elemento en la cola */
void insertarItem(char item, int numProd) {
    compartidoExt->buffer.cola[compartidoExt->buffer.fin] = item;
    compartidoExt->buffer.numElem++;
    
    printf(" Prod %d: Introdujo %c en la posición %d\n", numProd, item, compartidoExt->buffer.fin);
    
    compartidoExt->buffer.fin = (compartidoExt->buffer.fin +1) % N; //Lo incrementamos de forma modular.
}

int main(int argc, char** argv) {
    // Paso 1: Se comprueba que se introduce el número de argumentos adecuado y de formato adecuado por línea de comandos
    int numProd;
    if(argc != 2){
        perror("Error: no ha insertado el número requerido de argumentos. Formato aceptado: ./exe <num_prod> ");
        exit(EXIT_FAILURE);
    }else{
        if((numProd=atoi(argv[1]))==0){
            if(strcmp(argv[1], "0")==0){
                perror("Error: el número de productores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_prod> ");
            }else{
                perror("Error: el parámetro insertado no es un número entero. Formato aceptado: ./exe <num_prod> ");
            }
            exit(EXIT_FAILURE);
        }
        if(numProd<0){
            perror("Error: el número de productores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_prod> ");
            exit(EXIT_FAILURE);
        }
    }
    
    // Paso 2: Se mapean las memorias compartidas.
    /* Se crea, abre y ajusta el tamaño del fichero para la memoria compartida entre productores y consumidores, y se mapea su memoria en la 
     * estructura local compartidoExt.
     * A continuación se mapea la memoria compartida (interna) entre los distintos productores 
     */
    // Mapeo de memorias compartidas
    int file_compartido; 
    unlink("file_compartido");
    if((file_compartido= open("file_compartido", O_CREAT | O_RDWR, 0666))== -1){
        perror("Error al crear el archivo de memoria compartida");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(file_compartido, sizeof(CompartidoExt))== -1){
        perror("Error al ajustar el tamaño de la memoria compartida");
        exit(EXIT_FAILURE);
    }
    if((compartidoExt = mmap(NULL, sizeof(CompartidoExt), PROT_READ | PROT_WRITE, MAP_SHARED, file_compartido, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if((compartidoInt= mmap(NULL, sizeof(CompartidoInt), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
        perror("Error al ejecutar el mmap");
        exit(EXIT_FAILURE);
    }

    // Paso 3: Se inicializan variables necesarias, la barrera para poder emplearla entre procesos distintos
    // y los mutexes y variables de condición que se emplearán entre procesos y entre productor y consumidor
    compartidoExt->numProd= numProd;
    // Se inicializa la cola
    compartidoExt->buffer.inicio = 0;
    compartidoExt->buffer.fin= 0;
    compartidoExt->buffer.numElem= 0;

    // Se inicializan mutexes, variables de condición, barrera y sus atributos
    pthread_mutexattr_t mutex_atrib;
    pthread_condattr_t cond_atrib;
    pthread_barrierattr_t barrera_atrib;
    pthread_mutexattr_init(&mutex_atrib);
    pthread_condattr_init(&cond_atrib);
    pthread_barrierattr_init(&barrera_atrib);
    pthread_mutexattr_setpshared(&mutex_atrib, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cond_atrib, PTHREAD_PROCESS_SHARED);
    pthread_barrierattr_setpshared(&barrera_atrib, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&compartidoExt->mutex, &mutex_atrib);
    pthread_cond_init(&compartidoExt->condp, &cond_atrib);
    pthread_cond_init(&compartidoExt->condc, &cond_atrib);
    pthread_barrier_init(&compartidoInt->barrera, &barrera_atrib, compartidoExt->numProd);

    // Paso 4: Se inicia el proceso de producción
    /* Se pide el nombre del fichero a emplear para la lectura de items y el tiempo para el sleep() de cada uno, 
     * se crean los procesos productores requeridos y, hasta que no se haya insertado este valor para cada uno, 
     * no se inicia con la producción de items
     */
    for(int i=0; i < numProd; i++){
        // Se pide al usuario el nombre del fichero de lectura y el tiempo para el sleep()
        char fichero[100];
        int T;
        printf("Productor %d) Fichero a emplear: ", i+1);
        scanf(" %s", fichero);
        printf("\t     Tiempo a dormir el productor: ");
        scanf(" %d", &T);

        // Se crea el proceso hijo productor
        pid_t pid=fork();
        if(pid==0){
            //Sincronizamos a los productores
            pthread_barrier_wait(&compartidoInt->barrera); 
            
            // Se abre en modo lectura el fichero de items del proceso hijo
            FILE *fp;
            char item= '\0';
            if((fp=fopen(fichero,"r"))== NULL){
                perror("Error: no se pudo abrir el archivo indicado en modo lectura");
                exit(EXIT_FAILURE);
            }
            
            // Proceso de producción
            /* Solamente se leerán caracteres alfanuméricos del fichero, por lo que cuando se llegue
             * al final de este, para notificar el fin de la producción de este proceso, se produce 
             * un asterico como item. Esto implica que el proceso de producción del hijo continuará 
             * hasta que se produzca un asterisco
             */
            while (item != '*') {
                // Simula tiempo de producción
                sleep(rand()%T); 

                // Se lee un caracter del fichero de lectura y se devuelve un item
                item = generarItem(fp);
                // Se comprueba que el item devuelto sea un caracter alfanumérico o un asterico de finalización
                if((item>=48 && item<=57) || (item>=65 && item<=90) || (item>=97 && item<=122) || item=='*'){
                    // Bloquea el acceso a la región crítica
                    pthread_mutex_lock(&compartidoExt->mutex);  
                    while(compartidoExt->buffer.numElem == N) {
                        // Mientras el buffer compartido está lleno, espera a que se libere espacio
                        pthread_cond_wait(&compartidoExt->condp, &compartidoExt->mutex);   
                    }
                    // Se inserta el item en el buffer compartido
                    insertarItem(item, i+1);
                    
                    // Despierta a un proceso consumidor y libera el acceso a región crítica
                    pthread_cond_signal(&compartidoExt->condc);
                    pthread_mutex_unlock(&compartidoExt->mutex); 
                }
            }
            
            // Se cierra el fichero de lectura y se finaliza el proceso
            fclose(fp);
            exit(0);
        }
    }
    
    // Paso 5: Se espera a que los procesos hijos terminen
    for(int i=0; i < numProd; i++){
        wait(NULL);
    }

    // Paso 6: Se destruyen la barrera
    /* Aunque ha sido el productor quien creó e inicializó los mutexes y variables de condición,
     * el consumidor que continúa en ejecución aún requiere de su uso, por lo que será este
     * último el que se encargará de destruírlos al finalizar.
     */
    pthread_barrier_destroy(&compartidoInt->barrera);

    // Paso 7: Se desmapean las memorias compartidas y se cierra el fichero compartido
    munmap(compartidoExt, sizeof(CompartidoExt));
    munmap(compartidoInt, sizeof(CompartidoInt));
    close(file_compartido);

    return 0;
}

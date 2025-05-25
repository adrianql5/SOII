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
    //Buffer
    Buffer buffer;
    // Variables
    int numProd;
} CompartidoExt;

/* Estructura compartida solamente entre consumidores */
typedef struct{
    // Barrera
    pthread_barrier_t barrera;
    // Variables
    int contProd;
} CompartidoInt;

/* Variables globales */
CompartidoExt *compartidoExt; 
CompartidoInt *compartidoInt;
int numCons;

/* Función para retirar un elemento de la cola y escribirlo en un fichero */
char retirarItem(FILE *fp, int numCons) {
    if(compartidoExt->buffer.numElem==0) return '\0';

    char item = compartidoExt->buffer.cola[compartidoExt->buffer.inicio];
    compartidoExt->buffer.cola[compartidoExt->buffer.inicio]='\0';
    compartidoExt->buffer.numElem--;
    
    printf("Cons %d: Retiró %c de la posición %d\n", numCons, item, compartidoExt->buffer.inicio);
    
    if(item !='*'){
        putc(item, fp);
    }

    compartidoExt->buffer.inicio = (compartidoExt->buffer.inicio+1) % N;
    return item;
}

int main(int argc, char** argv) {
    // Paso 1: Se comprueba que se introduce el número de argumentos adecuado y de formato adecuado por línea de comandos
    if(argc != 2){
        perror("Error: no ha insertado el número requerido de argumentos. Formato aceptado: ./exe <num_cons> ");
        exit(EXIT_FAILURE);
    }else{
        if((numCons=atoi(argv[1]))==0){
            if(strcmp(argv[1], "0")==0){
                perror("Error: el número de consumidores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_cons> ");
            }else{
                perror("Error: el parámetro insertado no es un número entero. Formato aceptado: ./exe <num_cons> ");
            }
            exit(EXIT_FAILURE);
        }
        if(numCons<0){
            perror("Error: el número de consumidores a crear debe de ser mayor que 0. Formato aceptado: ./exe <num_cons> ");
            exit(EXIT_FAILURE);
        }
    }

    // Paso 2: Se mapean las memorias compartidas.
    /* Se abre el fichero para la memoria compartida entre productores y consumidores, y se mapea su memoria en la 
     * estructura local compartidoExt.
     * A continuación se mapea la memoria compartida (interna) entre los distintos consumidores 
     */
    int file_compartido;
    if((file_compartido = open("file_compartido", O_RDWR))==-1){
        perror("Error al abrir el archivo de memoria compartida");
        exit(EXIT_FAILURE);
    }
    if((compartidoExt = mmap(NULL, sizeof(CompartidoExt), PROT_READ | PROT_WRITE, MAP_SHARED, file_compartido, 0))==MAP_FAILED){
        perror("Error al mapear la memoria compartida");
        exit(EXIT_FAILURE);
    }
    if((compartidoInt= mmap(NULL, sizeof(CompartidoInt), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
        perror("Error al ejecutar el mmap");
        exit(EXIT_FAILURE);
    }

    // Paso 3: Se inicializan variables necesarias y la barrera para poder emplearla entre procesos distintos
    compartidoInt->contProd= 0;

    pthread_barrierattr_t barrera_atrib;
    pthread_barrierattr_setpshared(&barrera_atrib, PTHREAD_PROCESS_SHARED);
    pthread_barrier_init(&compartidoInt->barrera, &barrera_atrib, numCons);

    // Paso 4: Se inicia el proceso de consumo
    /* Se pide el tiempo para el sleep() de cada uno, se crean los procesos consumidores requeridos 
     * y, hasta que no se haya insertado este valor para cada uno, no se inicia con el consumo de items
     */
    for(int i=0; i < numCons; i++){
        // Se pide al usuario que inserte el tiempo para sleep()
        int T;
        printf("Consumidor %d) Tiempo a dormir el consumidor: ", i+1);
        scanf(" %d", &T);

        // Se crea el proceso hijo consumidor
        pid_t pid=fork();
        if(pid==0){
            //Sincronizamos a los consumidores
            pthread_barrier_wait(&compartidoInt->barrera); 

            // Se crea el nombre del fichero propio del consumidor hijo y se crea en modo escritura el fichero
            FILE *fp;
            char fichero [100];
            sprintf(fichero, "fileCons_%d.txt", i+1);
            if((fp=fopen(fichero, "w")) ==NULL){
                perror("Error: no se pudo crear el fichero para el consumidor");
                exit(EXIT_FAILURE); 
            }

            // Proceso de consumo
            /* Al recibir un asterisco por parte de un productor, se incrementa la variable compartida entre
             * concumidores contProd
             * Mientras esta variable no iguale el número de productores creados (se guardó en la estructura
             * compartida por el productor padre), no se finaliza con el consumo 
             */
            while (compartidoInt->contProd != compartidoExt->numProd) {
                // Simula tiempo de consumo
                sleep(rand()%T); 
                // Se comprueba si finalizó el proceso de consumo al despertarse del sleep(), para evitar que se
                // queden bloqueados en el mutex
                if(compartidoInt->contProd != compartidoExt->numProd){
                    break;
                }
                // Bloquea el acceso a la región crítica
                pthread_mutex_lock(&compartidoExt->mutex); 
                while(compartidoExt->buffer.numElem == 0){
                    // Espera que haya elementos en la cola 
                    pthread_cond_wait(&compartidoExt->condc, &compartidoExt->mutex); 
                    // Para evitar que consumidores hijos se queden bloqueados aquí cuando no haya más items
                    // que consumir, se comprueba la condición del bucle while
                    if(compartidoInt->contProd == compartidoExt->numProd){
                        break;
                    }
                } 

                // Se extrae el primer item del buffer y se escribe en el fichero propio
                char item=retirarItem(fp, i+1);
                
                // Si el item es un asterisco, se incrementa el contador contProd
                if(item == '*') compartidoInt->contProd++;

                // Despierta a un proceso productor y libera el acceso a región crítica
                pthread_cond_signal(&compartidoExt->condp);  
                pthread_mutex_unlock(&compartidoExt->mutex);
            }

            // Al terminar un consumidor hijo de consumir, despierta a los posibles consumidores hijos bloqueado
            pthread_cond_signal(&compartidoExt->condc);  

            // Se cierra el fichero propio de escritura y se finaliza el proceso
            fclose(fp);
            exit(0);
        }
    }
      
    // Paso 5: Se espera a que los procesos hijos terminen
    for(int i=0; i < numCons; i++){
        wait(NULL);
    }

    // Paso 6: Se destruyen los mutexes y barrera
    /* Como el consumidor finaliza después del productor,aún necesita poder acceder a los mutexes
     * y variables de condición, por lo que se encarga al finalizar de destruírlos, en lugar del
     * productor
     */
    pthread_cond_destroy(&compartidoExt->condp);
    pthread_cond_destroy(&compartidoExt->condc);
    pthread_mutex_destroy(&compartidoExt->mutex);
    pthread_barrier_destroy(&compartidoInt->barrera);

    // Paso 7: Se desmapean las memorias compartidas y se cierra el fichero compartido
    munmap(compartidoExt, sizeof(CompartidoExt));
    munmap(compartidoInt, sizeof(CompartidoInt));
    close(file_compartido);

    return 0;
}

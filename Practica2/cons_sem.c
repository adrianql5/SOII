/*
 * Código escrito por:
 * Joel Míguez Castelo
 * Adrián Quiroga Linares
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>

#define ITER 60 //Numero de iteraciones del consumidor
#define N 8 //Tamaño del buffer compartido

// Estructura del buffer compartido
struct Buffer {
    char pila[N]; //Pila de caracteres
    int tope; //Indice del tope actual de la pila
};

//Funcion para imprimir la string
void imprimirCadena(char *str, int longitud) {
    if (str == NULL || longitud <= 0) {
        printf("Cadena vacía o nula.\n");
        return;
    }

    printf("Contenido de str: ");
    for (int i = 0; i < longitud; i++) {
        printf("%c", str[i]);
    }
    printf("\n");
}

//Funcion para retirar un elemento del buffer compartido
char removeItem(struct Buffer *buffer) {
    printf("Retirando %c de la posicion %d\n", buffer->pila[buffer->tope], buffer->tope);
    return buffer->pila[buffer->tope];
}

//Funcion para almacenar un elemento consumido en la cadena local
void consumeItem(char **str, char item, int indice) {
    char *temp = realloc(*str, sizeof(char) * (indice + 1));
    if (!temp) {
        perror("Error en realloc");
        free(*str);
        exit(EXIT_FAILURE);
    }
    else{
        *str = temp;
        (*str)[indice] = item;
    }
}

int main() {
    // Arbrimos el archivo compartido
    int fd = open("/dev/shm/buffer", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error al abrir memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapeo del buffer compartido en la memoria del proceso
    struct Buffer *buffer = mmap(NULL, sizeof(struct Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Inicalizamos variables locales
    char *str = NULL;
    int iteracion = 0;
    char item;

    //Abrimos los semáforos
    sem_t *mutex=sem_open("MUTEX", 0);
    if(mutex==SEM_FAILED){
        perror("Error en mutex");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }

    sem_t *vacias=sem_open("VACIAS", 0);
    if(vacias==SEM_FAILED){
        perror("Error en vacias");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }

    sem_t *llenas=sem_open("LLENAS", 0);
    if(llenas==SEM_FAILED){
        perror("Error en llenas");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }



    for(int i=0; i<ITER; i++){
        //Simulamos tiempo de espera aleatorio especificado en el enunciado
        sleep(rand()%3);

        /*
        * Espera a que haya elementos en el buffer
        * Si hay 0 posiciones llenas espera
        */
        sem_wait(llenas);

        //Si la región crítica está ocupada espera
        sem_wait(mutex);

        //Región crítica
        item = removeItem(buffer);

        //sleep(rand%3); //tiempo de espera aletorio especificado en el enunciado
        buffer->tope--;
    
        //Indicamos al productor que el consumidor sale de la región crítica
        sem_post(mutex);
        /*
        * Indicamos el produtor que hay una posicion vacía
        * por si estaba esperando a que hubiese un hueco libre
        */
        sem_post(vacias);

        consumeItem(&str, item, iteracion);
        iteracion++;
    }

    //Imprimos la string
    imprimirCadena(str, iteracion);
    free(str);

    //Cerramos los semáforos
    sem_close(mutex);
    sem_close(vacias);
    sem_close(llenas);

    //Liberamos recursos asociados
    munmap(buffer, sizeof(struct Buffer));
    close(fd);
    
    // Desvinculamos los semáforos
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    return 0;
}



/*
 * Código escrito por:
 * Joel Míguez Castelo
 * Adrián Quiroga Linares
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>

#define ITER 60 //Numero de iteraciones
#define N 8 //Tamaño de la pila

struct Buffer {
    char pila[N]; //Pila que almacena los caracteres
    int tope; // Indice que indica el tope actual de la pila
};

// Funcion que imprime una string
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

//Funcion que genera un item
char producirItem() {
    return 'A' + (rand() % 26);
}

// Funcion que inserta el item en el buffer compartido
void insertarItem(struct Buffer *buffer, char item) {
    int indice = buffer->tope;
    buffer->pila[indice+1] = item;
    printf("Introduciendo %c en la posicion %d\n",item,indice+1);

}

int main() {
    //Eliminamos los semaforos previos por si quedaron vinculados por accidente anteriormente
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    srand(time(NULL));

    //Abrimos el archivo compartido
    int fd = open("/dev/shm/buffer", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error al abrir archivo de memoria compartida");
        exit(EXIT_FAILURE);
    }

    //Ajustamos el tamaño de la memoria compartida
    if (ftruncate(fd, sizeof(struct Buffer)) == -1) {
        perror("Error al ajustar tamaño de memoria compartida");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Mapeamos la memoria compartida
    struct Buffer *buffer = mmap(NULL, sizeof(struct Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Inicalizamos el buffer
    buffer->tope = -1;
    char item;
    char *str = NULL;
    int iteracion = 0;

    //Creamos los semáforos
    sem_t *mutex=sem_open("MUTEX", O_CREAT, 0666, 1);
    if(mutex==SEM_FAILED){
        perror("Error en mutex");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }

    sem_t *vacias=sem_open("VACIAS", O_CREAT, 0666, N);
    if(vacias==SEM_FAILED){
        perror("Error en vacias");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }

    sem_t *llenas=sem_open("LLENAS", O_CREAT, 0666, 0);
    if(llenas==SEM_FAILED){
        perror("Error en llenas");
        free(str);
        munmap(buffer, sizeof(struct Buffer));
        close(fd);
        exit(EXIT_FAILURE); 
    }

    for(int i=0; i<ITER; i++) {
        item = producirItem(); //producimos el item

        sleep(rand()%3); //espera especificada en el enunciado

        char *temp = realloc(str, sizeof(char) * (iteracion + 1));
        if (!temp) {
            perror("Error en realloc");
            free(str);
            munmap(buffer, sizeof(struct Buffer));
            close(fd);
            exit(EXIT_FAILURE);
        }
        else{
            str = temp;
            str[iteracion] = item;
            iteracion++;
        }

        //Si hay 0 posiciones vacías espera.
        sem_wait(vacias);

        //Si la region crítica está ocupada espera
        sem_wait(mutex);

        //Inicio de la región crítica
        insertarItem(buffer, item);

        sleep(rand()%3); //espera especificada ne el enunciado

        buffer->tope++;

        //Inidica al consumido que abanadona la region critica
        sem_post(mutex);

        /*
        * Indica al consumidor que hay una posicion
        * más llena por si estaba esperando
        */
        sem_post(llenas);
    }

    //Imprimimos la string
    imprimirCadena(str, iteracion);
    free(str);
    
    //Cerramos los semaforos
    sem_close(mutex);
    sem_close(vacias);
    sem_close(llenas);

    //Liberamos recusos asociados
    munmap(buffer, sizeof(struct Buffer));
    close(fd);

    //Desvinculamos semaforos
    sem_unlink("VACIAS");
    sem_unlink("LLENAS");
    sem_unlink("MUTEX");

    return 0;
}


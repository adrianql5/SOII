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

#define N 8 //Tamaño de la pila del buffer

int control = 1; //Variable de control para salir del programa

//Estructura compartida entre procesos
struct Buffer {
    char pila[N]; //Pila que contiene los items
    int tope; //Posicion del tope actual de la pila
};

// Manejador de SIGINT que desactiva la variable de control para salir del bucle
void manejador(int señal) {
    if(señal==SIGINT) control = 0;  

}

// Funcion para imprimir la string
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

// Funcion para guardar un elemento consumido en la string local
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

int main(int argc, char **argv) {
    signal(SIGINT, manejador);//Asignamos le manejador a la señal SIGINT

    if(argc<2){
        perror("Uso ./prod pausa\n");
        exit(EXIT_FAILURE);
    }

    int pausa=atoi(argv[1]); //variable que define el tiempo del sleep

    //Abrimos el archivo compartido
    int fd = open("/dev/shm/buffer", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error al abrir memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapeamos la memoria compartida en el espacio del proceso
    struct Buffer *buffer = mmap(NULL, sizeof(struct Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Inicializamos variables locales
    char *str = NULL;
    int iteracion = 0;
    char item;

    while (control) {
        //sleep(1); //sleep para probar el funcionamiento del codigo

        // Espera activa mientras el buffer esté vacío
        while (buffer->tope < 0 && control);
        
        if(buffer->tope>=0){
            
            // Retiramos un elemento del buffer
            item = removeItem(buffer); 
            
            sleep(pausa);

            /* Region critica ya que se accede a 
            * la variable compartida tope y si se actualiza 
            * en este momento, se puede perder la 
            * informacion de un elemento que se esta consumiendo
            */
            
            buffer->tope--;

            //Fin de la region crítica

            //Guardamos el elemento consumido
            consumeItem(&str, item, iteracion);
            iteracion++;
        }
    }

    //Imprimimos la string
    imprimirCadena(str, iteracion);

    //Liberamos recursos
    free(str);
    munmap(buffer, sizeof(struct Buffer));
    close(fd);

    return 0;
}


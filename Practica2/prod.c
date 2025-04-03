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

#define N 8 //Tamaño de la pila

int control = 1;// Variable de control para salir del bucle principal

//Estructura compartida entre procesos
struct Buffer {
    char pila[N]; //Pila que contiene los elementos
    int tope; //Indice del tope actual de la pila
};

// Manejador de SIGINT que desactiva la variable de control para salir del bucle
void manejador(int señal) {
    if(señal==SIGINT) control = 0;  
}

//Funcion que imprime una string
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


//Funcion para producir un item aleatorio
char producirItem() {
    return 'A' + (rand() % 26);//se genera un caracter aleatorio
}

//Funcion para insertar un elemento en el buffer
void insertarItem(struct Buffer *buffer, char item) {
    int indice = buffer->tope;
    buffer->pila[indice+1] = item; //Insertamos en la posición correspondiente
    printf("Introduciendo %c en la posicion %d\n", item, indice+1);
}

int main(int argc, char **argv) {
    signal(SIGINT, manejador);// Asignamos el manejador de la señal SIGINT 
    srand(time(NULL));

    if(argc<2){
        perror("Uso ./prod pausa\n");
        exit(EXIT_FAILURE);
    }

    int pausa=atoi(argv[1]);//Variable para decidir cuanto durará el sleep

    //Abrimos el archivo compartido
    int fd = open("/dev/shm/buffer", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error al abrir archivo de memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Ajustamos el tamaño del archivo de memoria compartida
    if (ftruncate(fd, sizeof(struct Buffer)) == -1) {
        perror("Error al ajustar tamaño de memoria compartida");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Mapeamos la memoria compartida en el espacio del proceso
    struct Buffer *buffer = mmap(NULL, sizeof(struct Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //Incializamos los valores del buffer
    buffer->tope = -1;
    char item;
    char *str = NULL;
    int iteracion = 0;

    while (control) {
        //sleep(2); //sleep para probar el funcionamiento del codigo
        // Producimos un nuevo ítem
        item = producirItem();
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

        // Espera activa mientras el buffer esté lleno
        while(buffer->tope >= N - 1 && control);


        if(buffer->tope <N-1){
            insertarItem(buffer, item); //Si hay espacio insertamos el elemento
            /*
            * Espera colocada en la seccion crítica,
            * puesto que accedemos a la variable compartida tope
            * y esta puede ser modificada por el consumidor antes
            * de incrementarla.
            */
            sleep(pausa);

            buffer->tope++;

            //Fin de la seccion crítica
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

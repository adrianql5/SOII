/*
 * Código escrito por:
 * Joel Míguez Castelo
 * Adrián Quiroga Linares
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define N 8    // Tamaño de la pila
#define ITER 60 // Número de iteraciones

// Estructura compartida entre el productor y el consumidor
struct Buffer {
    char pila[N]; // Pila de caracteres
    int tope;     // Índice del tope de la pila
} buffer; // Declarado como variable global

// Semáforos
sem_t mutex;   // Controla el acceso a la memoria compartida
sem_t llenas;  // Cuenta los elementos en la pila
sem_t vacias;  // Cuenta los espacios disponibles en la pila

char strProd[ITER]; // Array para almacenar los caracteres generados
char strCons[ITER]; // Array para almacenar los caracteres consumidos


// Función para generar un carácter aleatorio
char generarItem() {
    return 'A' + (rand() % 26); // Genera una letra entre 'A' y 'Z'
}

// Función para insertar un elemento en la pila
void insertarItem(char item) {
    buffer.pila[++(buffer.tope)] = item;
    printf("Productor: Introdujo %c en la posición %d\n", item, buffer.tope);
}

// Función para retirar un elemento de la pila
char retirarItem() {
    char item = buffer.pila[buffer.tope--];
    printf("Consumidor: Retiró %c de la posición %d\n", item, buffer.tope + 1);
    return item;
}

// Hilo productor
void *productor(void *arg) {
    int index = 0;  // Índice para almacenar en str

    for (int i = 0; i < ITER; i++) {
        sleep(rand() % 3); // Simula tiempo de producción

        char item = generarItem();
        strProd[index++] = item; // Almacena el carácter generado

        sem_wait(&vacias);  // Espera espacio vacío
        sem_wait(&mutex);   // Bloquea el acceso a la memoria compartida
        
        insertarItem(item);

        sem_post(&mutex);   // Libera la memoria compartida
        sem_post(&llenas);  // Indica que hay un elemento disponible
    }

    strProd[index] = '\0'; // Termina la cadena
    printf("Productor produjo: %s\n", strProd);

    pthread_exit(NULL);
}

// Hilo consumidor
void *consumidor(void *arg) {
    int index = 0;  // Índice para almacenar en str

    for (int i = 0; i < ITER; i++) {
        sleep(rand() % 3); // Simula tiempo de consumo

        sem_wait(&llenas);  // Espera que haya elementos en la pila
        sem_wait(&mutex);   // Bloquea el acceso a la memoria compartida

        char item = retirarItem();
        strCons[index++] = item; // Almacena el carácter consumido

        sem_post(&mutex);   // Libera la memoria compartida
        sem_post(&vacias);  // Indica que hay espacio disponible
    }

    strCons[index] = '\0'; // Termina la cadena
    printf("Consumidor consumió: %s\n", strCons);

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    buffer.tope = -1; // Inicializar la pila vacía

    // Inicializar semáforos
    sem_init(&mutex, 0, 1);
    sem_init(&llenas, 0, 0);
    sem_init(&vacias, 0, N);

    // Crear hilos de productor y consumidor
    pthread_t t_prod, t_cons;
    pthread_create(&t_prod, NULL, productor, NULL);
    pthread_create(&t_cons, NULL, consumidor, NULL);

    // Esperar a que los hilos terminen
    pthread_join(t_prod, NULL);
    pthread_join(t_cons, NULL);

    // Destruir los semáforos
    sem_destroy(&mutex);
    sem_destroy(&llenas);
    sem_destroy(&vacias);

    return 0;
}


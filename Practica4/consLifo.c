/*
*   Codigo escrito por
*
*   Adrián Quiroga Linares
*
*   Manuel Pérez Pazos
*/

#include <assert.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define TAM_BUFFER 5
#define NOMBRE_COLA_ENTRADA "/BUFFER_ENTRADA"
#define NOMBRE_COLA_SALIDA "/BUFFER_SALIDA"

mqd_t cola_entrada;   // Cola desde la que se leen los datos enviados por el productor
mqd_t cola_salida;    // Cola a la que se envían señales de disponibilidad para nuevos datos
char *ruta_archivo;   // Ruta del archivo donde se almacenará la salida
int retardo_max;      // Tiempo aleatorio máximo de espera (en microsegundos)

// Función que escribe un carácter en el archivo especificado
void escribir_en_archivo(int descriptor_archivo, char caracter)
{
    if (write(descriptor_archivo, &caracter, 1) != 1) {
        perror("Error al escribir en archivo");
        exit(EXIT_FAILURE);
    }
}

// Función que implementa el comportamiento del consumidor
void consumidor(void)
{
    char caracter_leido = 0;
    unsigned int prioridad;
    int archivo_salida;

    // Abre el archivo donde se almacenarán los datos recibidos
    if ((archivo_salida = open(ruta_archivo, O_CREAT | O_TRUNC | O_WRONLY, 0600)) == -1) {
        perror("Error al abrir archivo");
        exit(EXIT_FAILURE);
    }

    // Inicializa la cola de disponibilidad con señales indicando espacios libres
    for (int i = 0; i < TAM_BUFFER; i++) {
        mq_send(cola_salida, &caracter_leido, 1, 0);
    }

    // Ciclo principal donde se reciben datos y se escriben en el archivo
    while (1) {
        mq_receive(cola_entrada, &caracter_leido, 1, &prioridad);

        if (caracter_leido == EOF)
            break;

        usleep(rand() % retardo_max + 1);

        if (mq_send(cola_salida, &caracter_leido, 1, 0) == -1) {
            perror("Error al enviar señal a cola de salida");
            break;
        }

        escribir_en_archivo(archivo_salida, caracter_leido);

        usleep(rand() % retardo_max + 1);
    }

    close(archivo_salida);
    mq_close(cola_entrada);
    mq_close(cola_salida);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <archivo_salida> <retardo_max_us>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL)); // Inicializa generador de números aleatorios

    ruta_archivo = argv[1];
    retardo_max = atoi(argv[2]);

    cola_entrada = mq_open(NOMBRE_COLA_ENTRADA, O_RDONLY);
    cola_salida = mq_open(NOMBRE_COLA_SALIDA, O_WRONLY);

    if (cola_entrada == -1 || cola_salida == -1) {
        fprintf(stderr, "Error: asegurate de que el productor esté en ejecución\n");
        perror("Error al abrir colas");
        exit(EXIT_FAILURE);
    }

    consumidor();

    // Cierre adicional por seguridad
    mq_close(cola_entrada);
    mq_close(cola_salida);

    return EXIT_SUCCESS;
}


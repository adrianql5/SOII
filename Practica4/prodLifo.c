/*
*   Codigo escrito por
*
*   Adrián Quiroga Linares
*
*   Manuel Pérez Pazos
*/



#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define TAM_BUFFER 5
#define NOMBRE_COLA_ENTRADA "/BUFFER_ENTRADA"
#define NOMBRE_COLA_SALIDA "/BUFFER_SALIDA"

mqd_t cola_envio;   // Cola por la que se envían caracteres al consumidor
mqd_t cola_retorno; // Cola por la que se reciben señales de espacio disponible

int prioridad = 0;  // Prioridad que aumenta para mantener orden de envío

// Función principal del productor: lee archivo y envía datos al consumidor
void ejecutar_productor(const char *ruta_entrada, int retardo_max)
{
    char caracter;
    FILE *archivo_entrada = fopen(ruta_entrada, "r");

    if (!archivo_entrada) {
        perror("Error al abrir archivo de entrada");
        exit(EXIT_FAILURE);
    }

    do {
        caracter = fgetc(archivo_entrada);
        usleep(rand() % retardo_max + 1);

        char espacio_disponible;
        if (mq_receive(cola_retorno, &espacio_disponible, sizeof(espacio_disponible), NULL) == -1) {
            perror("Error al recibir señal de espacio disponible");
            break;
        }

        usleep(rand() % retardo_max + 1);

        if (mq_send(cola_envio, &caracter, sizeof(caracter), caracter == EOF ? 0 : ++prioridad) == -1) {
            perror("Error al enviar carácter al consumidor");
            break;
        }

    } while (caracter != EOF);

    fclose(archivo_entrada);
    mq_close(cola_envio);
    mq_close(cola_retorno);
}

int main(int argc, char *argv[])
{
    srand(time(NULL)); // Semilla aleatoria para los retardos

    if (argc != 3) {
        printf("Uso: %s <archivo_entrada> <tiempo_max_espera_us>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int retardo_max = atoi(argv[2]);
    struct mq_attr atributos_cola;

    atributos_cola.mq_maxmsg = TAM_BUFFER;
    atributos_cola.mq_msgsize = sizeof(char);
    atributos_cola.mq_flags = 0;

    // Elimina colas antiguas si ya existían
    mq_unlink(NOMBRE_COLA_ENTRADA);
    mq_unlink(NOMBRE_COLA_SALIDA);

    // Crea nuevas colas de mensajes para comunicación entre procesos
    cola_envio = mq_open(NOMBRE_COLA_ENTRADA, O_CREAT | O_WRONLY, 0666, &atributos_cola);
    cola_retorno = mq_open(NOMBRE_COLA_SALIDA, O_CREAT | O_RDONLY, 0666, &atributos_cola);

    if (cola_envio == -1 || cola_retorno == -1) {
        perror("Error al crear o abrir las colas de mensajes");
        exit(EXIT_FAILURE);
    }

    ejecutar_productor(argv[1], retardo_max);

    return EXIT_SUCCESS;
}




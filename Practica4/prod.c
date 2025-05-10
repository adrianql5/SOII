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

#define TAM_MAX_COLA 5

mqd_t cola_envio;    // Cola para enviar caracteres (productor)
mqd_t cola_recepcion; // Cola para recibir señales de espacio libre (consumidor)

void ejecutar_productor(const char *ruta_archivo, int tiempo_max)
{
    char caracter;
    FILE *archivo = fopen(ruta_archivo, "r");

    if (!archivo) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    do {
        caracter = fgetc(archivo);
        usleep(rand() % tiempo_max + 1); // Retardo aleatorio hasta tiempo_max

        // Esperar confirmación de espacio disponible
        char confirmacion;
        if (mq_receive(cola_recepcion, &confirmacion, sizeof(confirmacion), NULL) == -1) {
            perror("Error en mq_receive (esperando espacio)");
            break;
        }

        // Enviar carácter leído
        if (mq_send(cola_envio, &caracter, sizeof(caracter), 0) == -1) {
            perror("Error en mq_send (enviando carácter)");
            break;
        }
    } while (caracter != EOF);

    fclose(archivo);
    mq_close(cola_envio);
    mq_close(cola_recepcion);
}

int main(int argc, char *argv[])
{
    srand(time(NULL)); // Inicializa la semilla para aleatoriedad

    if (argc != 3) {
        fprintf(stderr, "Uso correcto: %s <archivo_entrada> <tiempo_max>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int intervalo = atoi(argv[2]);

    struct mq_attr propiedades_cola;
    propiedades_cola.mq_maxmsg = TAM_MAX_COLA;
    propiedades_cola.mq_msgsize = sizeof(char);

    // Limpieza previa de colas existentes
    mq_unlink("/COLA_ENVIO");
    mq_unlink("/COLA_RECEPCION");

    // Creación de las colas
    cola_envio = mq_open("/COLA_ENVIO", O_CREAT | O_WRONLY, 0777, &propiedades_cola);
    cola_recepcion = mq_open("/COLA_RECEPCION", O_CREAT | O_RDONLY, 0777, &propiedades_cola);

    if (cola_envio == -1 || cola_recepcion == -1) {
        perror("No se pudo abrir las colas de mensajes");
        exit(EXIT_FAILURE);
    }

    ejecutar_productor(argv[1], intervalo);

    return EXIT_SUCCESS;
}


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

#define TAM_BUFFER 5

mqd_t cola_lectura;    // Cola para recibir caracteres del productor
mqd_t cola_control;    // Cola para enviar señales de espacio disponible al productor

char *ruta_salida;     // Ruta del archivo donde se escriben los datos
int retardo_maximo;    // Tiempo máximo de espera aleatoria entre consumos (en microsegundos)

// Función auxiliar para guardar un carácter en el archivo destino
void guardar_en_archivo(int descriptor, char letra)
{
    if (write(descriptor, &letra, 1) != 1) {
        perror("Error al escribir en archivo");
        exit(EXIT_FAILURE);
    }
}

// Función principal encargada del proceso de consumo
void iniciar_consumidor(void)
{
    char simbolo = 0;
    int archivo_salida;

    // Abrir archivo destino en modo escritura, creación y truncado
    archivo_salida = open(ruta_salida, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (archivo_salida == -1) {
        perror("No se pudo abrir el archivo de salida");
        exit(EXIT_FAILURE);
    }

    // Inicializar cola de control indicando espacios disponibles
    for (int i = 0; i < TAM_BUFFER; i++) {
        mq_send(cola_control, &simbolo, sizeof(simbolo), 0);
    }

    // Bucle principal de recepción y escritura
    while (1) {
        mq_receive(cola_lectura, &simbolo, sizeof(simbolo), NULL);

        if (simbolo == EOF) {
            break;
        }

        mq_send(cola_control, &simbolo, sizeof(simbolo), 0);

        guardar_en_archivo(archivo_salida, simbolo);

        usleep(rand() % retardo_maximo + 1);
    }

    close(archivo_salida);
    mq_close(cola_lectura);
    mq_close(cola_control);
}

int main(int argc, char *argv[])
{
    cola_lectura = mq_open("/COLA_ENVIO", O_RDONLY);    // Se conecta a la cola del productor
    cola_control = mq_open("/COLA_RECEPCION", O_WRONLY); // Se conecta a la cola de control

    if (cola_lectura == -1 || cola_control == -1) {
        fprintf(stderr, "Error: Asegúrese de ejecutar primero el productor\n");
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (argc < 3) {
        fprintf(stderr, "Uso correcto: %s <archivo_salida> <retardo_microsegundos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ruta_salida = argv[1];
    retardo_maximo = atoi(argv[2]);

    iniciar_consumidor();

    // Doble cierre por seguridad
    mq_close(cola_lectura);
    mq_close(cola_control);

    return EXIT_SUCCESS;
}


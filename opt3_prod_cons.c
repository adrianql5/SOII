// Miembros equipo: Adrián Quiroga Linares, Lucía Pérez González

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

#define N 8       // Tamaño del buffer principal
#define M 5       // Tamaño del buffer numérico
#define ITER 60   // Número de iteraciones (no utilizado en este ejemplo)

// Estructura para el buffer numérico
struct BufferNumerico {
    char cola[M];
    int inicio;
    int fin;
    int numElem;
} bufferNumerico;

// Buffer principal compartido entre productor y consumidor intermedio
struct Buffer {
    char cola[N];
    int inicio;
    int fin;
    int numElem;
} buffer;

// Estructura que guarda los datos de cada hilo
typedef struct {
    char fichero[100]; // Nombre del fichero a abrir/crear (si procede)
    int T;             // Tiempo a dormir (para simular retardo)
    int numHilo;       // Identificador del hilo (para mostrar mensajes)
} Datos;

// Mutex y variables de condición para el buffer principal
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc = PTHREAD_COND_INITIALIZER;
pthread_cond_t condp = PTHREAD_COND_INITIALIZER;

// Mutex y variables de condición para el buffer numérico
pthread_mutex_t mutexNum = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condcNum = PTHREAD_COND_INITIALIZER;
pthread_cond_t condpNum = PTHREAD_COND_INITIALIZER;

// Barrera para sincronizar todos los hilos
pthread_barrier_t barrera;

// Variables globales para controlar la finalización de los productores
int numProd;      // Número total de productores (definido desde argv[1])
int contProd = 0; // Número de productores finalizados (cada productor inserta '*' al finalizar)

// Función que simula la generación de un ítem leyendo de un fichero
char generarItem(FILE *fp) {
    int c = getc(fp);
    if(c == EOF) 
        return '*'; // Marcador de final
    // Si se lee el marcador '*', se devuelve '\0' para evitar confusión
    if(c == '*') 
        return '\0';
    return (char)c;
}

// Inserta un elemento en el buffer principal
void insertarItem(char item, Datos *datos) {
    buffer.cola[buffer.fin] = item;
    buffer.numElem++;
    printf("Productor %d: Inserta '%c' en la posición %d\n", datos->numHilo, item, buffer.fin);
    buffer.fin = (buffer.fin + 1) % N;
}

// Retira un elemento del buffer principal y, si no es '*', lo escribe en fichero
char retirarItem(FILE *fp, Datos *datos) {
    if (buffer.numElem == 0)
        return '\0';
    
    char item = buffer.cola[buffer.inicio];
    buffer.cola[buffer.inicio] = '\0';
    buffer.numElem--;
    printf("\tConsumidor Intermedio %d: Retira '%c' de la posición %d\n", datos->numHilo, item, buffer.inicio);
    buffer.inicio = (buffer.inicio + 1) % N;
    
    if (item != '*' && fp != NULL) {
        putc(item, fp);
    }
    
    return item;
}


// Inserta un dígito en el buffer numérico
void insertarItemNum(char item, Datos *datos) {
    bufferNumerico.cola[bufferNumerico.fin] = item;
    bufferNumerico.numElem++;
    printf("Consumidor Intermedio %d: Inserta '%c' en buffer numérico en la posición %d\n", datos->numHilo, item, bufferNumerico.fin);
    bufferNumerico.fin = (bufferNumerico.fin + 1) % M;
}

// Hilo productor: lee caracteres de un fichero y los inserta en el buffer principal.
void *productor(void *arg) {
    Datos *datosProd = (Datos*)arg;
    FILE *fp;
    
    if ((fp = fopen(datosProd->fichero, "r")) == NULL) {
        perror("Error: no se pudo abrir el fichero en modo lectura");
        free(datosProd);
        pthread_exit(NULL);
    }
    
    pthread_barrier_wait(&barrera); // Sincroniza inicio de todos los hilos
    
    char item = '\0';
    do {
        sleep(rand() % (datosProd->T + 1)); // Simula retardo de producción
        item = generarItem(fp);
        
        // Solo se inserta si el caracter es alfanumérico o es el marcador de final ('*')
        if (( (item >= '0' && item <= '9') || 
              (item >= 'A' && item <= 'Z') || 
              (item >= 'a' && item <= 'z') ) || item == '*') {
            
            pthread_mutex_lock(&mutex);
            while (buffer.numElem == N)
                pthread_cond_wait(&condp, &mutex);
            
            insertarItem(item, datosProd);
            pthread_cond_signal(&condc);
            pthread_mutex_unlock(&mutex);
        }
        
    } while (item != '*');
    
    fclose(fp);
    free(datosProd);
    pthread_exit(NULL);
}

// Hilo consumidor intermedio: retira ítems del buffer principal, los escribe en un fichero y, si son dígitos, los coloca en el buffer numérico.
void *consumidorIntermedio(void *arg) {
    Datos *datosCons = (Datos*)arg;
    FILE *fp;
    
    if ((fp = fopen(datosCons->fichero, "w")) == NULL) {
        perror("Error: no se pudo crear el fichero para el consumidor intermedio");
        free(datosCons);
        pthread_exit(NULL);
    }
    
    pthread_barrier_wait(&barrera);
    
    int finalizar = 0;
    while (!finalizar) {
        sleep(rand() % (datosCons->T + 1)); // Simula retardo de consumo
        
        pthread_mutex_lock(&mutex);
        while (buffer.numElem == 0) {
            // Si ya se han finalizado todos los productores, salimos
            if (contProd == numProd) {
                finalizar = 1;
                break;
            }
            pthread_cond_wait(&condc, &mutex);
        }
        
        if (!finalizar) {
            char item = retirarItem(fp, datosCons);
            // Si se encontró el marcador de final, se incrementa el contador
            if (item == '*') {
                contProd++;
            } else if (isdigit(item)) {
                // Si es dígito, se inserta en el buffer numérico
                pthread_mutex_lock(&mutexNum);
                while (bufferNumerico.numElem == M)
                    pthread_cond_wait(&condpNum, &mutexNum);
                insertarItemNum(item, datosCons);
                pthread_cond_signal(&condcNum);
                pthread_mutex_unlock(&mutexNum);
            }
            pthread_cond_signal(&condp);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    fclose(fp);
    free(datosCons);
    pthread_exit(NULL);
}

// Hilo consumidor final: retira dígitos del buffer numérico y los procesa (en este ejemplo, suma los dígitos).
void *consumidorFinal(void *arg) {
    Datos *datosFinal = (Datos*)arg;
    int sumaLocal = 0;
    
    pthread_barrier_wait(&barrera);
    
    while (1) {
        sleep(rand() % (datosFinal->T + 1)); // Simula retardo de consumo final
        
        pthread_mutex_lock(&mutexNum);
        while (bufferNumerico.numElem == 0)
            pthread_cond_wait(&condcNum, &mutexNum);
        
        char item = bufferNumerico.cola[bufferNumerico.inicio];
        bufferNumerico.cola[bufferNumerico.inicio] = '\0';
        bufferNumerico.numElem--;
        printf("\t\tConsumidor Final %d: Retira '%c' de la posición %d del buffer numérico\n", datosFinal->numHilo, item, bufferNumerico.inicio);
        bufferNumerico.inicio = (bufferNumerico.inicio + 1) % M;
        pthread_cond_signal(&condpNum);
        pthread_mutex_unlock(&mutexNum);
        
        // Si es marcador de fin, termina el hilo
        if (item == '*')
            break;
        else if (isdigit(item))
            sumaLocal += (item - '0');
    }
    
    printf("Consumidor Final %d: Suma total = %d\n", datosFinal->numHilo, sumaLocal);
    free(datosFinal);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int numConsInter, numConsFinal;
    
    if (argc != 4) {
        fprintf(stderr, "Error: se requieren 3 argumentos.\nFormato: ./exe <num_prod> <num_cons_inter> <num_cons_final>\n");
        exit(EXIT_FAILURE);
    }
    
    if ((numProd = atoi(argv[1])) <= 0) {
        fprintf(stderr, "Error: el primer parámetro debe ser un número entero positivo.\n");
        exit(EXIT_FAILURE);
    }
    if ((numConsInter = atoi(argv[2])) <= 0) {
        fprintf(stderr, "Error: el segundo parámetro debe ser un número entero positivo.\n");
        exit(EXIT_FAILURE);
    }
    if ((numConsFinal = atoi(argv[3])) <= 0) {
        fprintf(stderr, "Error: el tercer parámetro debe ser un número entero positivo.\n");
        exit(EXIT_FAILURE);
    }
    
    // Inicializar buffers
    buffer.inicio = buffer.fin = buffer.numElem = 0;
    bufferNumerico.inicio = bufferNumerico.fin = bufferNumerico.numElem = 0;
    contProd = 0;
    
    // La barrera sincronizará: productores + consumidores intermedios + consumidores finales
    int totalHilos = numProd + numConsInter + numConsFinal;
    if (pthread_barrier_init(&barrera, NULL, totalHilos) != 0) {
        perror("Error: no se pudo crear la barrera");
        exit(EXIT_FAILURE);
    }
    
    // Crear hilos productores
    pthread_t t_prod[numProd];
    for (int i = 0; i < numProd; i++) {
        Datos *datosProd = malloc(sizeof(Datos));
        datosProd->numHilo = i + 1;
        printf("Productor %d) Nombre del fichero de entrada: ", i + 1);
        scanf("%s", datosProd->fichero);
        printf("\tTiempo a dormir (T): ");
        scanf("%d", &datosProd->T);
        pthread_create(&t_prod[i], NULL, productor, datosProd);
    }
    
    // Crear hilos consumidores intermedios
    pthread_t t_consInter[numConsInter];
    for (int i = 0; i < numConsInter; i++) {
        Datos *datosCons = malloc(sizeof(Datos));
        datosCons->numHilo = i + 1;
        printf("Consumidor Intermedio %d) Tiempo a dormir (T): ", i + 1);
        scanf("%d", &datosCons->T);
        // Genera un nombre de fichero de salida para este consumidor intermedio
        sprintf(datosCons->fichero, "fileCons_%d.txt", datosCons->numHilo);
        pthread_create(&t_consInter[i], NULL, consumidorIntermedio, datosCons);
    }
    
    // Crear hilos consumidores finales
    pthread_t t_consFinal[numConsFinal];
    for (int i = 0; i < numConsFinal; i++) {
        Datos *datosFinal = malloc(sizeof(Datos));
        datosFinal->numHilo = i + 1;
        printf("Consumidor Final %d) Tiempo a dormir (T): ", i + 1);
        scanf("%d", &datosFinal->T);
        // No se requiere fichero para el consumidor final
        pthread_create(&t_consFinal[i], NULL, consumidorFinal, datosFinal);
    }
    
    printf("\n\t\tINICIO DE LA EJECUCIÓN\n");
    printf("---------------------------------------------------------\n");
    
    // Esperar a que terminen los productores
    for (int i = 0; i < numProd; i++) {
        pthread_join(t_prod[i], NULL);
    }
    
    // Esperar a que terminen los consumidores intermedios
    for (int i = 0; i < numConsInter; i++) {
        pthread_join(t_consInter[i], NULL);
    }
    
    // Una vez que han terminado los consumidores intermedios, se insertan marcadores de fin ('*')
    // en el buffer numérico para notificar a los consumidores finales
    pthread_mutex_lock(&mutexNum);
    for (int i = 0; i < numConsFinal; i++) {
        while (bufferNumerico.numElem == M)
            pthread_cond_wait(&condpNum, &mutexNum);
        bufferNumerico.cola[bufferNumerico.fin] = '*';
        bufferNumerico.numElem++;
        bufferNumerico.fin = (bufferNumerico.fin + 1) % M;
        pthread_cond_signal(&condcNum);
    }
    pthread_mutex_unlock(&mutexNum);
    
    // Esperar a que terminen los consumidores finales
    for (int i = 0; i < numConsFinal; i++) {
        pthread_join(t_consFinal[i], NULL);
    }
    
    // Destruir mutex, variables de condición y la barrera
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condc);
    pthread_cond_destroy(&condp);
    
    pthread_mutex_destroy(&mutexNum);
    pthread_cond_destroy(&condcNum);
    pthread_cond_destroy(&condpNum);
    
    pthread_barrier_destroy(&barrera);
    
    return 0;
}


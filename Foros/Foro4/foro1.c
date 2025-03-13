#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define iteraciones 1000

double suma = 0.0;
int M, T;

void *sum(void *arg){
    for (int j = * ((int*)arg); j <= M; j += T)
        suma += j; 
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <M> <T>\n", argv[0]);
        return EXIT_FAILURE;
    }

    M = atoi(argv[1]);
    T = atoi(argv[2]);

    if (M <= 0 || T <= 0) {
        fprintf(stderr, "M y T deben ser números positivos.\n");
        return EXIT_FAILURE;
    }

    pthread_t *hilos = malloc(T * sizeof(pthread_t));
    int *ids = malloc(T * sizeof(int));

    if (!hilos || !ids) {
        fprintf(stderr, "Error al asignar memoria.\n");
        return EXIT_FAILURE;
    }

    int fallos = 0;
    double tiempoT = 0.0;

    for (int i = 0; i < iteraciones; i++) {

        struct timeval inicio, fin;
        gettimeofday(&inicio, NULL);

        for (int i = 0; i < T; i++) {
            ids[i] = i;
            pthread_create(&hilos[i], NULL, sum, (void *)&ids[i]);
        }

        for (int i = 0; i < T; i++) {
            pthread_join(hilos[i], NULL);
        }

        gettimeofday(&fin, NULL);
        double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_usec - inicio.tv_usec) / 1e6;
        tiempoT += tiempo;

        if (suma != (M * (M + 1)) / 2) fallos++;

        suma = 0.0;
    }

    free(hilos);
    free(ids);

    float x = (fallos / (float)iteraciones) * 100;
    float y = (tiempoT / iteraciones);
    printf("%d;%d;%lf;%lf\n", M, T, x, y);

    return EXIT_SUCCESS;
}


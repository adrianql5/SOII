#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdatomic.h>

#define iteraciones 1000

double suma = 0.0;
int M, T;

int turno=0;
int interesado[2] = {0, 0};

void entrar_seccion_critica(int id) {
    int otro = 1 - id;
    interesado[id] = 1;
    turno = id;
    while (interesado[otro] && turno == id) {
        sched_yield();
    }
}

void salir_seccion_critica(int id) {
    interesado[id] = 0;
}

void *sum(void *arg) {
    int id = *((int *)arg);
    double sumaLocal = 0.0;
    
    for (int j = id; j <= M; j += T){
        sumaLocal += j;
    }
    
    entrar_seccion_critica(id);
    suma += sumaLocal;
    salir_seccion_critica(id);
    
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <M> \n", argv[0]);
        return EXIT_FAILURE;
    }

    M = atoi(argv[1]);
    T=2;
    if (M <= 0 || T != 2) { // La solución de Peterson solo funciona con 2 hilos
        fprintf(stderr, "M debe ser positivo y T debe ser 2 para garantizar exclusión mutua con Peterson.\n");
        return EXIT_FAILURE;
    }

    pthread_t hilos[2];
    int ids[2] = {0, 1};
    int fallos = 0;
    double tiempoT = 0.0;

    for (int i = 0; i < iteraciones; i++) {
        suma = 0.0;
        interesado[0] = 0;
        interesado[1] = 0;
        turno=0;
        struct timeval inicio, fin;
        gettimeofday(&inicio, NULL);

        for (int i = 0; i < 2; i++) {
            pthread_create(&hilos[i], NULL, sum, (void *)&ids[i]);
        }
        for (int i = 0; i < 2; i++) {
            pthread_join(hilos[i], NULL);
        }

        gettimeofday(&fin, NULL);
        double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_usec - inicio.tv_usec) / 1e6;
        tiempoT += tiempo;

        if (suma != (M * (M + 1)) / 2) {
            fallos++;
        }
    }

    float x = (fallos / (float)iteraciones) * 100;
    float y = (tiempoT / iteraciones);
    printf("%d;%lf;%lf\n", M, x, y);

    return EXIT_SUCCESS;
}


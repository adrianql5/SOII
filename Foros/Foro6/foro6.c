#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

double suma = 0.0;
int M;
int T;
int errores = 0;
int *bandera;
int *turno;

void entrar_seccion_critica(int i, int j) {
    bandera[i] = j;
    turno[j] = i;
    
    while (1) {
        bool otro_presente = false;
        for (int k = 0; k < T; k++) {
            if (k != i && bandera[k] >= j) {
                otro_presente = true;
                break;
            }
        }
        if (!otro_presente || turno[j] != i) {
            break;
        }
    }
}

void salir_seccion_critica(int i) {
    bandera[i] = -1;
}

void *sum(void *arg) {
    int i = *(int *)arg;
    for (int j = 0; j < T - 1; j++) {
        entrar_seccion_critica(i, j);
        
        // Sección crítica
        for (int l = i + 1; l <= M; l += T) {
            suma += l;
        }

        salir_seccion_critica(i);
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <M> <T>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    M = atoi(argv[1]);
    T = atoi(argv[2]);

    bandera = (int *)malloc(T * sizeof(int));
    turno = (int *)malloc((T - 1) * sizeof(int));

    pthread_t hilos[T];
    int indices[T];

    for (int i = 0; i < T; i++) {
        bandera[i] = -1;
    }
    for (int i = 0; i < T - 1; i++) {
        turno[i] = 0;
    }
    struct timeval inicio, fin;
    gettimeofday(&inicio, NULL);

    for (int i = 0; i < T; i++) {
        indices[i] = i;
        pthread_create(&hilos[i], NULL, sum, &indices[i]);
    }

    for (int i = 0; i < T; i++) {
        pthread_join(hilos[i], NULL);
    }

    gettimeofday(&fin, NULL);

    double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_usec - inicio.tv_usec) / 1e6;

    if (suma != M * (M + 1) / 2) {
        errores++;
    }

    printf("%d;%d;%ld;%d\n", M, T, tiempo, errores);

    free(bandera);
    free(turno);

    exit(EXIT_SUCCESS);
}


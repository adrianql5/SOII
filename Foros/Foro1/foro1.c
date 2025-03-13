#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

double suma=0.0;

int M;
int T;

void *sum( void *arg){
    for (int j= * ((int*)arg); j<=M; j+=T)
        suma +=j;
    pthread_exit(NULL);
}

int main(int argc, char ** argv){
    M=atoi(argv[1]);
    T=atoi(argv[2]);

    pthread_t hilos[T];
    int ids[T];

    for(int i=0; i<T; i++){
        ids[i]=i;
        pthread_create(&hilos[i], NULL, sum, (void*) &ids[i]);
    }
    for(int i=0; i<T; i++){
        pthread_join(hilos[i], NULL);
    }


    if(suma!= M*(M+1)/2) exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);

}

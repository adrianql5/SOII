#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_TESTS 10000


int main() {
    srand(time(NULL)); // Inicializo la semilla para números aleatorios

    double a, b, c, aux, res1, res2;
    int count = 0; // Inicializo el contador de errores en 0

    for (int i = 1; i < NUM_TESTS; i++) {
        // Genero tres números aleatorios en doble precisión.
        a = ((double)rand() / RAND_MAX) * 3.0 - 1.0;  
        b = ((double)rand() / RAND_MAX) * 3.0 - 1.0;
        c = ((double)rand() / RAND_MAX) * 3.0 - 1.0;

        // Calculo (a * b) * c
        aux = a * b;
        res1 = aux * c;
        
        // Calculo a * (b * c)
        aux = b * c;
        res2 = a * aux;

        // Compruebo si los resultados son diferentes
        if (res1 != res2) {
            // Imprimo los primeros 5 casos donde la asociatividad no se cumple
            // No pongo todos porque en double se producen muchos
            if (count < 5) {
                printf("Error en la asociatividad: a = %.15f, b = %.15f, c = %.15f\n", a, b, c);
            }
            count++; // Aumento el contador de errores
        }

    }

    // Imprimo el total de casos donde la asociatividad no se cumplió
    printf("Se encontraron %d casos donde la asociatividad no se cumplió.\n", count);
    return 0;
}


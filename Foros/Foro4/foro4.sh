#!/bin/bash

rm foro4.1.csv foro4.2.csv

limites_suma=(100 500 1000 5000 10000 20000)
num_hilos=(1 2 4 8 16 32 64)


num_nucleos=$(nproc)
echo "Número de núcleos en el sistema: $num_nucleos"

gcc -O0 -o suma_espera_sin_optimizacion foro4.c -lpthread
gcc -O3 -o suma_sin_optimizacion foro1.c -lpthread


for M in "${limites_suma[@]}"; do
    for T in "${num_hilos[@]}"; do
            ./suma_espera_sin_optimizacion "$M" "$T">>foro4.1.csv
            ./suma_sin_optimizacion "$M" "$T">>foro4.2.csv
    done
done


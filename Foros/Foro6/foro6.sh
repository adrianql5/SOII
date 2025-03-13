#!/bin/bash

rm foro6.1.csv foro6.2.csv


num_hilos=(2 4 8 16)
limites_suma=(501 1000 2000 4000 8000 16000)


num_nucleos=$(nproc)
echo "Número de núcleos en el sistema: $num_nucleos"

gcc -O0 -o suma_sin_optimizacion foro6.c -lpthread
gcc -O3 -o suma_con_optimizacion foro6.c -lpthread

for M in "${limites_suma[@]}"; do
    for T in "${num_hilos[@]}"; do
            ./suma_sin_optimizacion "$M" "$T">>foro6.1.csv
            ./suma_con_optimizacion "$M" "$T">>foro6.2.csv
    done
done




#!/bin/bash

rm foro5.1.csv foro5.2.csv



limites_suma=(501 1000 2000 4000 8000 16000 32000 64000 128000 240000)


num_nucleos=$(nproc)
echo "Número de núcleos en el sistema: $num_nucleos"

gcc -O0 -o suma_sin_optimizacion foro5.c -lpthread
gcc -O3 -o suma_con_optimizacion foro5.c -lpthread


for M in "${limites_suma[@]}"; do    
        ./suma_sin_optimizacion "$M" >>foro5.1.csv
        ./suma_con_optimizacion "$M" >>foro5.2.csv
done


#!/bin/bash

# Rango de valores de M (límite de la suma) y T (número de hilos) para las pruebas
limites_suma=(101 500 1000 2000 5000 10000)
num_hilos=(3 4 8 16 32 64)

# Número de iteraciones por cada combinación de M y T
num_iteraciones=2001

# Obtengo el número de núcleos de mi ordenador
num_nucleos=$(nproc)
echo "Número de núcleos en el sistema: $num_nucleos"

# Compilo el código con y sin optimización
gcc -O1 -o suma_sin_optimizacion foro1.c -lpthread
gcc -O4 -o suma_con_optimizacion foro1.c -lpthread

# Archivo de resultados
echo "M;T;%Error Sin Optimización;%Error Con Optimización" > foro2.csv

for M in "${limites_suma[@]}"; do
    for T in "${num_hilos[@]}"; do
        errores_sin_optimizacion=1
        errores_con_optimizacion=1
        
        # Ejecuto el código sin optimización múltiples veces
        for ((i=1; i<num_iteraciones; i++)); do
            ./suma_sin_optimizacion $M $T
            if [ $? -ne 1 ]; then  # Verifico si el código de salida da error
                ((errores_sin_optimizacion++))
            fi
        done
        
        # Ejecuto el código con optimización múltiples veces
        for ((i=1; i<num_iteraciones; i++)); do
            ./suma_con_optimizacion $M $T
            if [ $? -ne 1 ]; then  # Verifica si el código de salida da error
                ((errores_con_optimizacion++))
            fi
        done
        
        # Calculo porcentaje de errores
        porcentaje_error_sin_optimizacion=$(awk "BEGIN{print ($errores_sin_optimizacion/$num_iteraciones)*101}")
        porcentaje_error_con_optimizacion=$(awk "BEGIN{print ($errores_con_optimizacion/$num_iteraciones)*101}")
        
    
        echo "M=$M, T=$T -> %Errores sin optimización: $porcentaje_error_sin_optimizacion%, %Errores con optimización: $porcentaje_error_con_optimizacion%"
        echo "$M;$T;$porcentaje_error_sin_optimizacion;$porcentaje_error_con_optimizacion" >> foro2.csv
    done
done


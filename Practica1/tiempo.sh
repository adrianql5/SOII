#!/bin/bash
# Código hecho por Xabier Nóvoa y Adrián Quiroga
if [ $# -ne 1 ]; then
    echo "Uso: $0 MM:DD:AAAA"
    exit 1
fi

# Cálculo de diferencias de tiempo
fechaInicialSegundos=$(date -d "$1" +%s)
fechaActualSegundos=$(date +%s)

diferenciaSegundos=$((fechaActualSegundos - fechaInicialSegundos))

aniosTranscurridos=$((diferenciaSegundos / (365 * 24 * 3600)))
diasTranscurridos=$(((diferenciaSegundos % (365 * 24 * 3600)) / (24 * 3600)))
minutosTranscurridos=$((((diferenciaSegundos % (365 * 24 * 3600)) % (24 * 3600)) / 60))

echo "Desde la fecha $1 han pasado:"
echo "- $aniosTranscurridos años"
echo "- $diasTranscurridos días"
echo "- $minutosTranscurridos minutos"

# Cálculo de años bisiestos entre ambas fechas
anioInicial=$(date -d "$1" +%Y)
anioActual=$(date +%Y)

cantidadBisiestos=0
for ((anio=$anioInicial; anio<=anioActual; anio++)); do
    if (( (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0) )); then
        ((cantidadBisiestos++))
    fi
done

echo "Número exacto de años bisiestos entre ambas fechas: $cantidadBisiestos."

# Consideración del calendario juliano antes de 1582, donde hicieron un ajuste donde el día siguiente al 4 de octubre de 1582 paso a ser  de  día 15 de octubre de 1582
if [ $anioInicial -lt 1582 ]; then
    segundosError=$(((1582 - anioInicial) * (11 * 60 + 14)))
    mintutosError=$((segundosError/60))
    echo "Si consideramos el cambio del calendario Juliano al Gregoriano, debemos aumentar $segundosError segundos ($mintutosError minutos)."

    segundosConCorreccion=$((segundosError + diferenciaSegundos))

    aniosCorregidos=$((segundosConCorreccion / (365 * 24 * 3600)))
    diasCorregidos=$(((segundosConCorreccion % (365 * 24 * 3600)) / (24 * 3600)))
    minutosCorregidos=$((((segundosConCorreccion % (365 * 24 * 3600)) % (24 * 3600)) / 60))

    echo "Desde la fecha $1 han pasado (con corrección del calendario):"
    echo "- $aniosCorregidos años"
    echo "- $diasCorregidos días"
    echo "- $minutosCorregidos minutos"
fi


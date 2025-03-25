#!/bin/bash

#Verifica si el número de parámetros pasados al script es diferente de 2 ("$#" es el número de parámetros). Si es diferente, entra en el bloque then.
if [ "$#" -ne 2 ]; then 
    echo "Uso: $0 [-c | GET/POST | -t | -s | -o] ruta/al/archivo/access.log"
    exit 1
fi

# Verificar que el segundo parámetro es un archivo regular con permisos de lectura
if [ ! -f "$2" ] || [ ! -r "$2" ]; then
    echo "Error: El archivo $2 no existe o no tiene permisos de lectura."
    exit 1
fi

#Asigna el primer parámetro ($1) a la variable OPCION y el segundo parámetro ($2) a la variable ARCHIVO.
OPCION="$1"
ARCHIVO="$2"

case "$OPCION" in #Comienza una estructura de control case para manejar diferentes opciones que el usuario puede proporcionar.
    -c) # Mostrar los diferentes códigos de respuesta y su cantidad
        awk '{print $9}' "$ARCHIVO" | sort | uniq -c | sort -nr
        ;;
#Usa awk para extraer la novena columna de cada línea del archivo de log (que corresponde al código de respuesta HTTP), luego los ordena,
#cuenta las ocurrencias únicas con uniq -c y finalmente ordena los resultados en orden numérico descendente (sort -nr).
    
    GET|POST) # Contar accesos con GET/POST y código de respuesta 200
        FECHA=$(date "+%b %d %H:%M:%S") #Obtiene la fecha y hora actual en formato "Mes Día Hora:Minuto:Segundo" y la guarda en la variable FECHA.
        COUNT=$(awk -v metodo="$OPCION" '$6 ~ metodo && $9 == 200 {count++} END {print count}' "$ARCHIVO")
        echo "$FECHA. Registrados $COUNT accesos tipo $OPCION con respuesta 200."
        ;;
#Usa awk para contar las líneas del archivo que corresponden al método HTTP ($6 es la columna del método) y que tienen un código de respuesta 200 ($9 == 200). 
#La variable metodo se pasa a awk para que busque el valor de OPCION (que es GET o POST).
    
    -t) # Contar los días sin acceso
        FIRST_DATE=$(awk '{print $4}' "$ARCHIVO" | sed 's/\[//' | cut -d: -f1 | sort | head -n1)
        LAST_DATE=$(awk '{print $4}' "$ARCHIVO" | sed 's/\[//' | cut -d: -f1 | sort | tail -n1)
        ALL_DATES=$(seq $(date -d "$FIRST_DATE" +%s) 86400 $(date -d "$LAST_DATE" +%s) | while read sec; do date -d @$sec +%d/%b/%Y; done)
        LOG_DATES=$(awk '{print $4}' "$ARCHIVO" | sed 's/\[//' | cut -d: -f1 | sort | uniq)
        NO_ACCESS_DAYS=$(comm -23 <(echo "$ALL_DATES") <(echo "$LOG_DATES") | wc -l)
        echo "Número de días sin acceso: $NO_ACCESS_DAYS"
        ;;
    -s) # Resumir total de datos enviados en KiB por cada mes
        awk '{month=substr($4,5,3); bytes=$10} bytes ~ /^[0-9]+$/ {data[month]+=bytes; count[month]++} END {for (m in data) printf "%d KiB sent in %s by %d accesses.\n", data[m]/1024, m, count[m]}' "$ARCHIVO"
        ;;
#Usa awk para extraer el mes (substr($4,5,3)) y el número de bytes enviados ($10). Suma los bytes por mes y cuenta el número de accesos por mes. 
#Finalmente, imprime el total de datos enviados por mes en KiB (dividiendo por 1024).
 
    -o) # Ordenar el archivo por bytes enviados de forma descendente
        sort -k10 -nr "$ARCHIVO" > access_ord.log
        echo "Archivo ordenado guardado en access_ord.log."
        ;;
#Usa el comando sort para ordenar el archivo access.log en función de la décima columna (que contiene el número de bytes enviados), de forma numérica y descendente.
#El resultado se guarda en el archivo access_ord.log.
    *)
        echo "Opción no válida. Uso: $0 [-c | GET/POST | -t | -s | -o] ruta/al/archivo/access.log"
        exit 1
        ;;
esac

#./accesos.sh -c ~/Documentos/SOII/P1/material_P1/log/access.log


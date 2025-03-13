#!/bin/bash
#1- cat /etc/passwd > ~/passwd_copia
#2- cat /ect/passwd >> ~/passwd_copia wc passwd_copia
# Código hecho por Xabier Nóvoa y Adrián Quiroga
l=$(wc -l < ~/passwd_copia)

sort -d ~/passwd_copia | uniq > /tmp/passwd_original

r=$(wc -l < /tmp/passwd_original)

x=$((l - r))

echo "Número de líneas duplicadas y eliminadas: $x"

if diff <(sort -d /tmp/passwd_original) <(sort -d /etc/passwd) > /dev/null; then
    echo "Los archivos son IGUALES."
else
    echo "Los archivos son DIFERENTES."
fi

 


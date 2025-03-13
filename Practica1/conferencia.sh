#! /bin/bash

#Código hecho por Xabier Nóvoa y Adrián Quiroga

origen=$1
destino=$2

#rmdir $2 descomentar 
# Comprobar si se han recibido dos parámetros
if [ "$#" -ne 2 ]; then
    echo "Se requieren exactamente dos parámetros."
    exit 1
fi

#Comprobar si existe el directorio origen y si se tienen permisos de lectura
if [ ! -d $origen ]; then
    echo "El directorio origen no existe."
    exit 1
elif [ ! -r $origen ]; then
      echo "No tienes permisos de lectura en el directorio origen."
    exit 1
fi

#Comprobar si existe el directorio origen y si se tienen permisos de escritura
if [ ! -d $destino ]; then
    echo "El directorio destino no existe."
    echo "Forma correcta: ./conferencia.sh path_origen path_destino"
    exit 1
elif [ ! -w $destino ]; then
      echo "No tienes permisos de escritura en el directorio origen."
      echo "Forma correcta: ./conferencia.sh path_origen path_destino"
    exit 1
fi

if [ -d "$destino/conferencia" ]; then
  echo "Ya existe un directorio con ese nombre"
  exit 1
fi

#Crear la carpeta conferencia dentro del directorio destino
mkdir "$destino/conferencia"


# Sacar las fechas de conferencias de todos los archivos
nombres_lista=()

for nombre in "$1"/*; do
  #Sacar solo la fecha
  subcarpeta="$(basename "$nombre" | cut -c 9-18)"
  
  # Comprobar si el nombre ya está en la lista
  if [[ ! " ${nombres_lista[@]} " =~ " ${subcarpeta} " ]]; then
    # Si no está, agregarlo a la lista
    nombres_lista+=("$subcarpeta")
  fi
done

# Creación de carpetas de las salas
for ((i = 20; i <= 50; i++)); do
  mkdir "$destino/conferencia/sala$i"
  #Creación de una carpeta para cada fecha aunque vayan a estar vacias (enunciado)
  for fecha in "${nombres_lista[@]}"
  do
    mkdir "$destino/conferencia/sala$i/$fecha"
  done
done


#Meter cada archivo en su directorio correspondiente
for nombre in $1/* 
do
  subcarpeta="$(basename "$nombre" | cut -c 9-18)" # Solo con el nombre de la carpeta base y los caracteres adecuados
  resolucion="$(basename "$nombre" | cut -c 26-27)"
  if [ "$resolucion" == "Fu" ]; then #En los casos Full HD no cuadra, se tiene que cambiar
    resolucion="Full HD"
  fi
  sala="$(basename "$nombre" | cut -c 6-7)"
  nombreNuevo="charla_$(basename "$nombre" | cut -c 20-24)" #Nuevo nombre del archivo

  echo "$subcarpeta,$resolucion,$sala,$nombreNuevo"

  mkdir -p "$destino/conferencia/sala$sala/$subcarpeta/$resolucion"
  cp "$nombre" "$destino/conferencia/sala$sala/$subcarpeta/$resolucion/$nombreNuevo" 
done












#!/bin/bash
# Código hecho por Xabier Nóvoa y Adrían Quiroga
# Comprobamos si se pasa un directorio como argumento, si no se pasa usamos el directorio actual
directorio=$1
# Si no existe el directorio, mostramos un mensaje de error
if [[ ! -d "$directorio" ]]; then
  echo "Error: El directorio '$directorio' no existe."
  exit 1
fi

# Función para ordenar según el número de caracteres del nombre de los ficheros
ordenar_por_caracteres() {
  echo "Ordenando por número de caracteres en los nombres de los ficheros:"
    ls $directorio | awk '{print length, $0}' | sort -n | cut -d' ' -f2-
 
}

# Función para ordenar según el orden alfabético de los nombres escritos al revés
ordenar_por_nombre_invertido() {
  echo "Ordenando por nombre de los ficheros (alfabéticamente, invertido):"
  ls "$directorio" | rev | sort | rev #rev lo que hace es invertir el orden de los caracteres
}

# Función para ordenar según el valor de los últimos 4 dígitos del inode
ordenar_por_inode() {
  echo "Ordenando por los últimos 4 dígitos del inode:"
  ls -i "$directorio" | awk '{print substr($1,length($1)-3,4), $0}' | sort -n 
}

# Función para ordenar según tamaño y grupos definidos por los permisos (rwx del propietario)
ordenar_por_tamano_y_permisos() {
  echo "Ordenando por tamaño y agrupado por permisos (rwx del propietario):"
  # substr(columna, caracter donde empieza, longitud de la cadena)
  ls -l "$directorio" | awk '{print substr($1, 2, 3), $5, $9}' | sort -k2,2n -k1,1
}


# Función para ordenar según el tamaño y agrupado por el mes de último acceso
# ls -l muestra la ultima fecha de modificacion. -lu la ultima de acceso
ordenar_por_acceso() {
  echo "Ordenando por tamaño y agrupado por mes de último acceso:"
  ls -lu "$directorio" | awk '{print $6, $5, $9}' | sort -k1,1 -k2,2
}

# Comprobamos los argumentos y ejecutamos la opción correspondiente
case "$2" in
  "-a")
    ordenar_por_caracteres
    ;;
  "-b")
    ordenar_por_nombre_invertido
    ;;
  "-c")
    ordenar_por_inode
    ;;
  "-d")
    ordenar_por_tamano_y_permisos
    ;;
  "-e")
    ordenar_por_acceso
    ;;
  *)
    echo "Opciones disponibles:"
    echo "-a: Ordenar por número de caracteres del nombre"
    echo "-b: Ordenar por el nombre alfabéticamente invertido"
    echo "-c: Ordenar por el valor de los últimos 4 dígitos del inode"
    echo "-d: Ordenar por tamaño y agrupado por los permisos rwx del propietario"
    echo "-e: Ordenar por tamaño y agrupado por el mes de último acceso"
    exit 1
    ;;
esac


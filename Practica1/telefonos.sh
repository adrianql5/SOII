#!/bin/bash
# Código hecho por Xabier Nóvoa y Adrián Quiroga
# Verificar los argumentos
if [ $# -eq 1 ]; then
  echo "Formato incorrecto. Ejemplo de uso:"
  echo "telefonos.sh -opcion nombre_archivo"
  echo "-a Crear una nueva agenda"
  echo "-e [nombre@numero] Registrar una nueva entrada"
  echo "-n [nombre] Buscar por nombre"
  echo "-t [telefono] Buscar por número de teléfono"
  echo "-m [nombre] [numeroNuevo] Modificar entrada"
  echo "-d [nombre] Borrar entrada"
  exit 1
fi

# Verificar permisos de archivo
if [[ ! -r "$2" || ! -w "$2" ]]; then
    echo "No tienes los permisos suficientes en el archivo. Asegúrate de que puedes leer y escribir en el archivo."
    exit 1
fi

# Comprobar si existe la agenda
if [[ $(head -n 1 "$2") != "nombre@numero" && "$1" != "-a" ]]; then
  echo "No se ha detectado una agenda. Primero crea una"
  echo "-a Crear una nueva agenda"
  exit 1
fi

# Opciones de caso
case $1 in
  "-a")
    echo "nombre@numero" > "$2"
    echo "Se ha creado la agenda"
    ;;
  
  "-e")
    if [ $# -eq 3 ]; then
      echo "$2" >> "$3"  # último parámetro siempre es el nombre del archivo
      echo "Se añadió la entrada a la agenda"
    else
      echo "Formato incorrecto para -e. Debe ser: -e [nombre@numero] nombre_archivo"
    fi
    ;;
  
  "-n")
    if [ $# -eq 3 ]; then
      echo "Resultados:"
      grep "$2" "$3"
    else
      echo "Formato incorrecto para -n. Debe ser: -n [nombre] nombre_archivo"
    fi
    ;;
  
  "-t")
    if [ $# -eq 3 ]; then
      echo "Resultados:"
      grep "$2" "$3"
    else
      echo "Formato incorrecto para -t. Debe ser: -t [telefono] nombre_archivo"
    fi
    ;;
  
  "-m")
    if [ $# -eq 4 ]; then
      sed -i "/$2/d" "$3"  # Eliminar la entrada con ese nombre
      echo "$2@$3" >> "$3"  # Añadir la entrada con ese nombre y el nuevo número
      echo "Se ha modificado la entrada"
    else
      echo "Formato incorrecto para -m. Debe ser: -m [nombre] [numeroNuevo] nombre_archivo"
    fi
    ;;
  
  "-d")
    if [ $# -eq 3 ]; then
      sed -i "/$2/d" "$3"  # Eliminar la entrada con ese nombre
      echo "Se ha eliminado la entrada"
    else
      echo "Formato incorrecto para -d. Debe ser: -d [nombre] nombre_archivo"
    fi
    ;;
  
  *)
    echo "Formato incorrecto. Ejemplo de uso:"
    echo "telefonos.sh -opcion nombre_archivo"
    echo "-a Crear una nueva agenda"
    echo "-e [nombre@numero] Registrar una nueva entrada"
    echo "-n [nombre] Buscar por nombre"
    echo "-t [telefono] Buscar por número de teléfono"
    echo "-m [nombre] [numeroNuevo] Modificar entrada"
    echo "-d [nombre] Borrar entrada"
    ;;
esac


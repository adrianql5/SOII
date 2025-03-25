#!/bin/bash
# Formato do aquivo: IP - - [Fecha:Hora Zona] "GET/POST URL" Respuesta Bytes

# Menu de axuda
if [[ $1 == "-h" ]]; then
    echo "Opciones disponibles:"
    echo "  -c     Muestra los diferentes códigos de respuesta sin repetición y cuántas veces aparece cada uno."
    echo "  -t     Muestra el número de días sin accesos al servidor desde la fecha del primer acceso hasta la fecha del último."
    echo "  GET    Cuenta el total de accesos tipo GET con respuesta 200."
    echo "  POST   Cuenta el total de accesos tipo POST con respuesta 200."
    echo "  -s     Resume el total de datos enviados (en KiB) por cada mes."
    echo "  -o     Ordena las líneas en orden decreciente según los bytes enviados y guarda el resultado en 'access_ord.log'."
    exit 0
fi

# Comprobacion do numero de parametros de entrada
if [[ $# -ne 2 ]]; then
    echo "Numero incorrecto de parametros :("
    echo "Uso: accesos.sh [GET/POST, -flag] ruta/access.log"
    exit 1
fi

# Comprobacion de que o segundo parametro sexa un arquivo con permisos de lectura
if [[ ! -f $2 || ! -r $2 ]]; then
    echo "El segundo parametro no es un archivo o no tiene permisos de lectura :("
    echo "Uso: accesos.sh [GET/POST, -flag] ruta/access.log"
    exit 1
fi

# Funcion que busca as liñas con resposta = 200 e GET/POST
# Ollo! $6 = \"GET ou \"POST
atoparGetPost(){
    modo="\"$1" # Engadimoslle as " para que coincida co campo $6
    count=$(awk -v modo="$modo" '$9 == 200 && $6 == modo {print}' "$2" | wc -l)
    modo=$(echo "$1" | tr -d '\"') # Eliminamoslle as " para imprimilo resultado
    echo "$(date +"%b %d %H:%M:%S"). Registrados $count accesos tipo $modo con respuesta 200."
}

# Funcion que calcula os datos enviados un mes dado en KiB
datosEnviados(){
    # Imprimimos o campo $4 que conten a data, e o campo $10, que conten os bytes enviados
    # Seleccionamos os que teñan o mes de interese e sumamos os bytes na variable bytes
    bytes=$(awk '{print $4, $10}' "$2" | grep "$1" | awk '{total += $2} END {print total}')
    kib=$((bytes / 1024)) # Conversion a  kibibytes
    num_accesos=$(awk '{print $4, $10}' "$2" | grep "$1" | wc -l)
    echo "$kib KiB sent in $1 by $num_accesos accesses."
}

# Switch case para as diferentes funcionalidades do programa
case $1 in
    -c) # O campo Resposta correspondese co argumento $9 de awk
        echo "Repeticiones Respuesta"
        awk '{print $9}' $2 | sort | uniq -c
        ;;
    -t) # Extraemos as datas inicial e final do arquivo
        t=$(head -n 1 "$2" | awk '{print $4}') # Data en formato dd/Mon/yyyy
        data_ini=${t:1:11} # A data correspondese cos caracteres 1-11 do campo 4 da liña
        t=$(tail -n 1 "$2" | awk '{print $4}') # Data en formato dd/Mon/yyyy
        data_fin=${t:1:11}
        # Iteramos sobre todas as datas dende a primeira ata a ultima
        count=0
        while [ "$data_ini" != "$data_fin" ]; do
            t=$(grep "$data_ini" $2)
            if [[ -z "$t" ]]; then
                ((++count))
            fi
            data_ini=$(echo "$data_ini" | sed 's/\// /g') # Sustituimos barras por espazos para que date recoñeza a data
            data_ini=$(LC_TIME=C date -d "$data_ini + 1 day" +%d/%b/%Y)
        done
        echo "Dias sin accesos: $count"
        ;;
    GET)
        atoparGetPost "GET" $2
        ;;
    POST)
        atoparGetPost "POST" $2
        ;;
    -s)
        datosEnviados "Jan" $2
        datosEnviados "Feb" $2
        datosEnviados "Mar" $2
        datosEnviados "Apr" $2
        datosEnviados "May" $2
        datosEnviados "Jun" $2
        datosEnviados "Jul" $2
        datosEnviados "Aug" $2
        datosEnviados "Sep" $2
        datosEnviados "Oct" $2
        datosEnviados "Nov" $2
        datosEnviados "Dec" $2
        ;;
    -o) # Ordena en funcion do campo 10 (Bytes) en orde decrecente
        sort -k10,10nr $2 > access_ord.log
        ;;
    *)
        echo "Opcion no valida :("
        echo "Para ver las opciones disponibles, introduzca la opcion -h"
        ;;
esac

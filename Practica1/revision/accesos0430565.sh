#!/bin/bash

opcionC(){
    respuesta=() #array que almacena los códigos de respuesta
    numRespuesta=() #array que almacena el número de veces que aparece cada código de respuesta
    while read -r linea #bucle para leer el archivo
    do
        codigo=$(cut -d'"' -f3- <<< "$linea") #almacena el codigo
        codigo="${codigo%\ *}" 
  
        cond=0
        for i in "${!respuesta[@]}" #bucle para comprobar si el código ya existe en el array
        do
            if test "${respuesta[$i]}" -eq "$codigo" #si existe se incrementa el contador
            then
                cond=1
                numRespuesta[$i]=$((${numRespuesta[$i]} + 1))
                break
            fi
        done

        if test $cond -eq 0 #si no existe se añade al array
        then
            respuesta+=("$codigo")
            numRespuesta+=("1")
        fi
    done < "$1"


    for i in "${!respuesta[@]}" #bucle para mostrar los resultados
    do
        echo "${respuesta[$i]} repetido ${numRespuesta[$i]} veces"
    done
}

opcionT(){
    fechas=() #array que almacena las fechas

    j=0
    while read -r linea #bucle para leer el archivo
    do
        ((j++))
        echo "$j"
        codigo=$(cut -d'[' -f2 <<< "$linea") #almacena la fecha sin formatear
        codigo="${codigo%%:*}"
        #echo "$codigo"
        
        fecha=$(date -d "$(echo $codigo | tr '/' ' ')" +"%s") #almacena la fecha formateada
        #echo "$fecha"

        cond=0
        for i in "${!fechas[@]}" #recorre el array para ver si la fecha ya esta almacenada
        do
            if test "${fechas[$i]}" == "$fecha" #si ya esta almacenada sale del bucle
            then
                cond=1
                break
            fi
        done

        if test $cond -eq 0 #si no esta almacenada la añade al array
        then    
            fechas+=("$fecha")
        fi

    done < "$1"

    inicio="${fechas[0]}" #guardamos la primera fecha y la pasamos a formato de día
    inicio=$(($inicio/3600/24)) 

    fin="${fechas[${#fechas[@]}-1]}" #guardamos la última fecha y la pasamos a formato de día
    fin=$(($fin/3600/24))   

    diferencia=$(($fin-$inicio)) #calculamos la diferencia entre la última y la primera fecha
    dias=$(($diferencia-${#fechas[@]})) #calculamos el numero de dias que no estan el array

    echo "Numero de dias sin acceso: $dias"


}

opcionPOST(){

    fecha=$(date +"%B %d %H:%M:%S") #alamacena la fecha y hora a la que se empieza a ejecutar el script
    
    numero=0 #contador de accesos POST con respuesta 200
    while read -r linea #bucle para leer el archivo
    do
        codigo=$(cut -d'"' -f3- <<< "$linea") #almacena el codigo de respuesta
        codigo="${codigo%\ *}"
        codigo=$(tr -d ' ' <<< "$codigo") 

        post=$(cut -d'"' -f2 <<< "$linea") #almacena el tipo de acceso
        post="${post%%\ *}"

        if test "$post" == "POST" #comprobamos si el acceso es POST
        then
            if test "$codigo" -eq 200 #comprobamos si el codigo de respuesta es 200
            then 
            ((numero++)) #aumentamos el contador
            fi
        fi
    done < "$1"

    echo "$fecha. Registrados $numero accesos POST con respuesta 200"
}

opcionGET(){

    fecha=$(date +"%B %d %H:%M:%S") #alamacena la fecha y hora a la que se empieza a ejecutar el script
    
    numero=0 #contador de accesos GET con respuesta 200
    while read -r linea #bucle para leer el archivo
    do
        codigo=$(cut -d'"' -f3- <<< "$linea") #almacena el codigo de respuesta
        codigo="${codigo%\ *}"
        codigo=$(tr -d ' ' <<< "$codigo")

        post=$(cut -d'"' -f2 <<< "$linea") #almacena el tipo de acceso
        post="${post%%\ *}"

        if test "$post" == "GET" #comprobamos si el acceso es GET
        then
            if test "$codigo" -eq 200 #comprobamos si el codigo de respuesta es 200
            then 
            ((numero++)) #aumentamos el contador
            fi
        fi
    done < "$1"

    echo "$fecha. Registrados $numero accesos GET con respuesta 200"
}

opcionS(){
    
    declare -a bytes_mes=(0 0 0 0 0 0 0 0 0 0 0 0) #array que almacena los bytes enviados por mes
    declare -a nombres=("Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec") #array que almacena los nombres de los meses
    declare -a accesos_mes=(0 0 0 0 0 0 0 0 0 0 0 0) #array que almacena el numero de accesos por mes
    
    while read -r linea #bucle para leer el archivo
    do
        mes=$(cut -d'/' -f2- <<< "$linea") #almacena el mes
        mes=$(cut -d'/' -f1 <<< "$mes")

        bytes=$(cut -d'"' -f3 <<< "$linea") #almacena el bytes enviados
        bytes=$(cut -d" " -f3 <<< "$bytes")
        
        
        if ! [[ "$bytes" =~ ^[0-9]+$ ]] #comprueba si el bytes enviados es un numero si no lo es suma 0
        then 
            bytes=0
	    fi
        
    
        case $mes in #dependiendo del mes suma los bytes enviados y añade un acceso
            Jan)
                bytes_mes[0]=$((bytes_mes[0] + bytes))  
                accesos_mes[0]=$((accesos_mes[0] + 1))
                ;;
            Feb)
                bytes_mes[1]=$((bytes_mes[1] + bytes))  
                accesos_mes[1]=$((accesos_mes[1] + 1))
                ;;
            Mar)
                bytes_mes[2]=$((bytes_mes[2] + bytes))  
                accesos_mes[2]=$((accesos_mes[2] + 1))
                ;;
            Apr)
                bytes_mes[3]=$((bytes_mes[3] + bytes))  
                accesos_mes[3]=$((accesos_mes[3] + 1))
                ;;
            May)
                bytes_mes[4]=$((bytes_mes[4] + bytes)) 
                accesos_mes[4]=$((accesos_mes[4] + 1))
                ;;
            Jun)
                bytes_mes[5]=$((bytes_mes[5] + bytes))  
                accesos_mes[5]=$((accesos_mes[5] + 1))
                ;;
            Jul)
                bytes_mes[6]=$((bytes_mes[6] + bytes))  
                accesos_mes[6]=$((accesos_mes[6] + 1))
                ;;
            Aug)
                bytes_mes[7]=$((bytes_mes[7] + bytes))  
                accesos_mes[7]=$((accesos_mes[7] + 1))
                ;;
            Sep)
                bytes_mes[8]=$((bytes_mes[8] + bytes))  
                accesos_mes[8]=$((accesos_mes[8] + 1))
                ;;
            Oct)
                bytes_mes[9]=$((bytes_mes[9] + bytes))  
                accesos_mes[9]=$((accesos_mes[9] + 1))
                ;;
            Nov)
                bytes_mes[10]=$((bytes_mes[10] + bytes))  
                accesos_mes[10]=$((accesos_mes[10] + 1))
                ;;
            Dec)
                bytes_mes[11]=$((bytes_mes[11] + bytes))  
                accesos_mes[11]=$((accesos_mes[11] + 1))
                ;;
            *)
                ;;
        esac
    
    done < "$1"
    
    for i in {0..11} #imprime el resultado
    do 
    	kiB=$((bytes_mes[$i] / 1024))
        echo "${kiB} KiB sent in ${nombres[$i]} by ${accesos_mes[$i]} acceses"
    done
    
}

opcionO(){
  sort -t ' ' -k10,10n "$1" > access_ord.log #ordena el archivo por el campo 10 contando que cada campo esta separado por un espacio

}


#Comprobar si se pasaron solo 2 parámetros  
if test $# -ne 2
then
    echo "Error: Se deben pasar 2 parámetros: $0 -opcion \"/rutaAlArchivo\""
    exit 1  
fi

#Comprobar si el primer parametro es un directorio con permisos de lectura
if test ! -f "$2"
then
	echo "Error: El primer parametro debe ser un archivo regular"
	exit 1
fi

if test ! -r "$2"
then
	echo "Error: El primer parametro debe ser un archivo regular con permisos de lectura"
	exit 1
fi

#Selector de opcion
case "$1" in
    -c)
        opcionC "$2"
        ;;
    -t)
        opcionT "$2"
        ;;
    GET)
        opcionGET "$2"
        ;;
    POST)
        opcionPOST "$2"
        ;;
    -s)
        opcionS "$2"
        ;;
    -o)
        opcionO "$2"
        ;;
    *)
        echo "Error: Opción no reconocida"
        echo "Usar: $0 {-c | -t | GET | POST | -s | -o} \"/rutaAlArchivo\""
        exit 1
        ;;
esac


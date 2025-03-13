#! /bin/bash
#Código hecho por Xabier Nóvoa Gómez y Adrián Quiroga
if [ "$#" -ne 2 ]; then
    echo "Uso: $0 [-c, GET/POST, -s] directorio_origen"
    exit 1
fi


if [ ! -f "$2" ] || [ ! -r "$2" ]; then 
    echo "Uso: $0 [-c, GET/POST, -s] directorio_origen"
    exit 1
fi


opcion=$1

case $opcion in
    "-c")
        echo "Codigos de respuesta:"
        #awk usa como separador por defecto el espacio
        #seleccionamos la novena columna que serian los codigos de
        #respuesta, los ordenamos e imprimimos sin repetcion
        #contando cuantas veces aparece (uniq -c)
        awk '{print $9}' $2 | sort | uniq -c 
        ;;

    "-t")
    # Calculamos las diferentes fechas en las que se accede    
    diasLog="$(awk -F '[:[]' '{print $2}' "$2" | uniq | wc -l)"
 
    # Miramos tanto el primer acceso como el ultimo
    #Despues tenemos que pasar la fecha a un formato adecuado para el comando date
    #Sed permite sustituir caracteres por otros: s(de sustituir)/patrón(meterle un escape de formato)/reemplazo(un espacio)/de manera global
    #Cambier / por espacios para el formato de date
    primerAcceso="$(awk -F '[:[]' '{print $2}' $2 | head -n 1 | sed 's/\// /g')"

    ultimoAcceso="$(awk -F '[:[]' '{print $2}' $2 | tail -n 1 | sed 's/\// /g')"
 
    #Calculamos los dias entre ambas fechas
    #El +%s pasa a segundos y realizamos la resta, después pasamos a días dividiento 
    #entre el número de segundos
    dias=$(expr \( $(date -d "$ultimoAcceso" +%s) - $(date -d "$primerAcceso" +%s) \) / 86400 + 1)
 
    # Si existiese alguna diferencia entre las fechas, habría 
    # algún día sin accesos
    diferenciaDias="$(expr $dias - $diasLog)"
  
    echo "Entre el primer acceso ($primerAcceso) y el último acceso ($ultimoAcceso) hubo $diferenciaDias dias en los que no se realizó ninguna conexión"
        ;;

    "GET")
        cuenta=$(awk '{print $6" "$9}' $2 | sort | uniq -c | awk '/GET 200/ {print $1}')
        
        fecha=$(date +"%b %d %H:%M:%S")  # Formato: Feb 12 11:10:31

        echo "$fecha. Registrados $cuenta accesos tipo GET con respuesta 200"
        ;;

    "POST")
        cuenta=$(awk '{print $6" "$9}' $2 | sort | uniq -c | awk '/POST 200/ {print $1}')

        fecha=$(date +"%b %d %H:%M:%S") # Formato: Feb 12 11:10:31

        echo "$fecha. Registrados $cuenta accesos tipo POST con respuesta 200"
            
        ;;

    "-s")   
        #Para sacar los meses 
        #awk -F '[" "\[/]' '{print $6}' ./access.log | sort -m | uniq
        for month in Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec; do
            numEntradas="$(awk "/\/$month\//" $2 | wc -l)" #Contar el número de líneas que tiene el archivo con cada mes
            resultadoMes="$(awk "/\/$month\// {s+=\$10} END {print int(s/1024)}" $2)" 
            #Sumar cada cada columna de datos de cada fila de cada mes
            echo "$resultadoMes KiB sent in $month by $numEntradas accesses"
         done
        ;;

    "-o")
        sort -n -t" " -k10 -o salida.log $2
        #-n para ordenar por número -t para establecer el separador y -k10 para elegir la columna 10
        echo "El resultado se ha guardado en el archivo salida.log"
        ;;

    *)
        ;;
esac

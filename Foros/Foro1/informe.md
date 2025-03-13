Objetivo del experimento:

    Evaluar el impacto del número de iteraciones, el número de hilos y los niveles de optimización del compilador en la probabilidad de que se presenten condiciones de carrera en el cálculo de la suma de enteros.

Metodología:

    El programa proporcionado tiene como objetivo calcular la suma de los primeros M enteros naturales. El cálculo se realiza utilizando T hilos. El valor correcto de la suma es calculado como M*(M+1)/2, y el programa devuelve un código de salida 0 si y solo si el resultado coincide con el valor esperado.
    
    Para el experimento usé un script que ejecuta el programa y redirige la salida a un archivo .csv para posteriormente elaborar gráficas. Este script realiza un seguimiento del número de errores, determinado por el código de salida del programa. Si no se especifican valores, el experimento se repite para diferentes valores de M (100, 500, 1000, 2000, 5000, 10000) y T (2, 4, 8, 16, 32, 64). El número total de repeticiones para cada dupla de valores posibles es de 2000.

Conclusiones:

    Se observa que a medida que aumenta el número de enteros a sumar y el número de hilos, la tasa de errores también aumenta. Esto es esperado, ya que un mayor número de sumandos genera más oportunidades para que ocurran condiciones de carrera, especialmente cuando los hilos acceden a la misma memoria compartida. El aumento de la cantidad de hilos también incrementa la probabilidad de que ocurran accesos concurrentes no sincronizados.

    Las optimizaciones del compilador tienen un impacto significativo en la tasa de errores.

Entorno de ejecución:

    Procesador: Intel Core i7-1360P (13ª generación, 16 hilos)
    Sistema operativo: Debian GNU/Linux 12
    Versión del núcleo: 6.12.12-amd64
    Plataforma gráfica: Wayland
    Memoria: 16GB RAM
    GPU: Intel Iris Xe Graphics
    Terminal: Kitty
    Shell: Zsh 5.9
                                 
                                                      

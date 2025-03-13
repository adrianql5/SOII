El valor más grande que se puede alcanzar para la variable suma daría en la situación ideal, donde no ocurren interferencias entre los hilos. En este caso, los hilos suman sus valores de manera secuencial sin sobrescribir los cálculos de otros hilos, resultando en el valor correcto de la suma, que depende de la cantidad de enteros.La fórmula para este valor correcto es:

$$\frac{M \times (M+1)}{2}$$

Este es el valor que obtendríamos si no hubiera carreras críticas, es decir, si cada hilo sumara su parte sin interferir con los demás.

El valor más pequeño dependerá de las carreras críticas y de cómo los hilos acceden a la variable suma de manera simultánea. Cuando varios hilos intentan actualizar la variable al mismo tiempo, parte de la suma se pierde, ya que los hilos sobrescriben el valor de suma sin sumar correctamente sus contribuciones previas. Si el número de hilos es igual \( M \), el primer hilo con identificador 0 podría ejecutarse después de los demás hilos, dejando la variable suma con un valor 0. En este caso, el hilo 0 sería el último en actualizar suma, y si los otros hilos ya han hecho su trabajo, el valor final de suma sería incorrecto.

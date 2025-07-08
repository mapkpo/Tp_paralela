## Consigna

Paralelizar con MPI el código de mergesort.c https://w3.cs.jmu.edu/lam2mo/cs470_2017_01/files/mergesort.c  
1. randomize() Se ejecuta en el nodo 0 y se reparten los datos equitativamente entre el total de nodos.
2. Cada nodo calcula el histograma de sus datos.
3. Cada nodo ordena sus datos y luego se juntan y ordenan con merge().

Para compilar y ejecutar el codigo ingresar los siguientes comandos:
```
    mpicc -o mergeSortConMPI mergeSortConMPI.c
    mpirun -np 4 ./mergeSortConMPI 10000
```
- 4: cantidad de procesos paralelos MPI.

- 10000: cantidad total de números enteros aleatorios que el programa generará en el proceso maestro y distribuirá entre los procesos.

Cada proceso recibirá: 10000 / 4 = 2500 números.

Se pueden modificar los valores, solo estan como referencia. 

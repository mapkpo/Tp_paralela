# Tp final de programación paralela 2025


## Consigna
Edge-detecting 
Partiendo del código de ejemplo en: https://github.com/steven-chien/DD2360-HT19.git
Implementar los 3 pasos (grayscale, gauss filter y sobel filter)
 
Deben realizarse 4 implementaciones:
* CPU serie
* CPU con openMP
* CUDA con global memory
* CUDA con shared memory
 
Analizar y documentar las diferencias de performance entre las 4 implementaciones
 
MPI.
Paralelizar con MPI el código de mergesort.c 
1. randomize() Se ejecuta en el nodo 0 y se reparten los datos equitativamente entre el total de nodos.
2. Cada nodo calcula el histograma de sus datos.
3. Cada nodo ordena sus datos y luego se juntan y ordenan con merge().

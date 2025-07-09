Para usar openmp se debio convertir el codigo a un binario de C en vez de uno cuda para poder usar gcc y linkear la libreria de openmp 

se agregó la directiva #pragma omp parallel for en los ciclos for de las funciones:
cpu_grayscale, cpu_gaussian, cpu_sobel


para compilar usamos:
``
gcc -fopenmp -O3 cpuConOpenMp.c -o cpuConOpenMp -lm
``


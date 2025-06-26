Para usar openmp se debio convertir el codigo a un binario de C en vez de uno cuda para poder usar gcc y linkear la libreria de openmp 


para compilar usamos:
``
gcc -fopenmp -o3 cpuConOpenMp.c -o cpuConOpenMp -lm
``


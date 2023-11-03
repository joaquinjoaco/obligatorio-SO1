// --------------------------------------------
// Obligatorio Sistemas Operativos
// Tecnologo Informatico 2023
//
// repostero.c
//
// Autor: Joaquin Gomez Leites (5.398.934-6)
// --------------------------------------------

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    // ============ RECURSO COMPARTIDO (HELADERA) ============
    int shm_fd;     // Descriptor de archivo de memoria compartida.
    int *heladera;  // Puntero para acceder a la memoria compartida.

    // Abrimos la region de memoria compartida (la heladera compartida entre cocineros y mozos).
    shm_fd = shm_open("heladera", O_CREAT | O_RDWR, 0666);
    // Configuramos el tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd, 4096);
    int SIZE = 4096;
    // Mapeamos la region de memoria a un puntero para poder acceder a ella.
    heladera = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    // ========================================================
    // SEMAFOROS DE HELADERA Y REPOSTERO.
    // Codigo del repostero.

    // --------------------------
    // DEBEN DE LLAMARSE AL FINAL.
    // Cerramos el descriptor de archivo de memoria compartida.
    close(shm_fd);
    // Eliminamos la región de memoria compartida.
    shm_unlink("heladera");

    return 0;
}
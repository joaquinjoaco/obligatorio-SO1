// --------------------------------------------
// Obligatorio Sistemas Operativos
// Tecnologo Informatico 2023
//
// mozos.c
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
    // ============ RECURSOS COMPARTIDO (MESADA, HELADERA) ============

    int SIZE = 4096;  // Tamaño de la memoria compartida 4096 bytes.
    // ============ RECURSO COMPARTIDO (MESADA) ============
    int shm_fd_mesada;  // Descriptor de archivo de memoria compartida.
    int *mesada;        // Puntero para acceder a la memoria compartida.
    // =========== RECURSO COMPARTIDO (HELADERA) ===========
    int shm_fd_heladera;  // Descriptor de archivo de memoria compartida.
    int *heladera;        // Puntero para acceder a la memoria compartida.

    // Abrimos las regiones de memoria compartida.
    shm_fd_mesada = shm_open("mesada", O_CREAT | O_RDWR, 0666);
    shm_fd_heladera = shm_open("heladera", O_CREAT | O_RDWR, 0666);

    // Configuramos los tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd_mesada, 4096);
    ftruncate(shm_fd_heladera, 4096);

    // Mapeamos las regiones de memoria a los punteros para poder acceder a ellas.
    mesada = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_mesada, 0);
    heladera = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_heladera, 0);

    // ========================================================

    // SEMAFOROS DE HELADERA, MESADA Y MOZO.

    // Codigo del mozo.

    // --------------------------
    // DEBEN DE LLAMARSE AL FINAL.
    // Cerramos los descriptores de archivo de memoria compartida.
    close(shm_fd_mesada);
    close(shm_fd_heladera);
    // Eliminamos las regiones de memoria compartida.
    shm_unlink("heladera");
    shm_unlink("mesada");

    return 0;
}
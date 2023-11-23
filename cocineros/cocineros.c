// --------------------------------------------
// Obligatorio Sistemas Operativos
// Tecnologo Informatico 2023
//
// cocineros.c
//
// Autor: Joaquín Gómez Leites (5.398.934-6)
// --------------------------------------------

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main() {
    // Unlinks de semáforos por las dudas.
    sem_unlink("sem_mesada");
    sem_unlink("sem_cocinero");
    sem_unlink("sem_platos");

    // ============ RECURSOS COMPARTIDOS (MESADA, PLATOS DEL DÍA) ============

    int SIZE = 4096;  // Tamaño de la memoria compartida 4096 bytes.

    // ============ RECURSO COMPARTIDO (MESADA) ============
    int shm_fd_mesada;  // Descriptor de archivo de memoria compartida.
    int *mesada;        // Puntero para acceder a la memoria compartida.
    // ========= RECURSO COMPARTIDO (PLATOS DEL DIA) =========
    int shm_fd_platos;    // Descriptor de archivo de memoria compartida.
    int *platos_del_dia;  // Puntero para acceder a la memoria compartida (heladera).

    // Abrimos las region de memoria compartida (la mesada compartida entre cocineros y mozos).
    mesada = shm_open("mesada", O_CREAT | O_RDWR, 0666);
    shm_fd_platos = shm_open("platos", O_CREAT | O_RDWR, 0666);

    // Configuramos los tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd_mesada, 4096);
    ftruncate(shm_fd_platos, 4096);

    // Mapeamos las regiones de memoria a los punteros para poder acceder a ellas.
    mesada = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_mesada, 0);
    *mesada = 0;  // La mesada abre con la mesada vacía.

    platos_del_dia = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_platos, 0);
    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día.

    // ========================================================
    // ===== Semáforos COCINERO, MESADA Y PLATOS DEL DIA =====
    sem_t *sem_cocinero, *sem_mesada, *sem_platos;
    // El valor del semáforo de la heladera representa la cantidad de espacios libres en la heladera.
    sem_cocinero = sem_open("sem_cocinero", O_CREAT, 0644, 25);  // sem cocinero inicilizado en 0 (la cantina abre con la heladera llena. 0 espacios libres).
    // El valor del semáforo de la mesada representa la cantidad de espacios libres en ella.
    sem_mesada = sem_open("sem_mesada", O_CREAT, 0644, 27);  // sem mesada contador inicilizado en 27 (la cantina abre con la mesada vacía. Todos los espacios libres).
    sem_platos = sem_open("sem_platos", O_CREAT, 0644, 1);   // sem platos del día inicializado en 1.

    // ========================================================

    // ================ Codigo del cocinero ===================
    // TODO: CICLO DE LA CANTINA 12 MEDIODIA.
    // Tomamos el valor actual del semáforo de la heladera.

    // ================ FINAL DEL PROGRAMA ====================
    // Cerramos los descriptores de archivo de memoria compartida.
    close(shm_fd_mesada);
    close(shm_fd_platos);

    // Eliminamos las regiones de memoria compartida.
    shm_unlink("mesada");
    shm_unlink("platos");

    // Cerramos los semáforos.
    sem_close(sem_cocinero);
    sem_unlink("sem_cocinero");

    sem_close(sem_mesada);
    sem_unlink("sem_mesada");

    sem_close(sem_platos);
    sem_unlink("sem_platos");
    // ========================================================

    return 0;
}
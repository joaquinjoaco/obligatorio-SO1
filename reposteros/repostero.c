// --------------------------------------------
// Obligatorio Sistemas Operativos
// Tecnologo Informatico 2023
//
// repostero.c
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

void preparar_flan() {
    // Prepara un flan (dummy).
    printf("Preparando flan...\n");
    // srand(time(NULL));
    // int randomSleep = rand() % 10;
    // sleep(randomSleep);
    sleep(1);
    printf("Flan listo.\n");
}

int main() {
    // Unlinks de semáforos por las dudas.
    sem_unlink("sem_repostero");
    sem_unlink("sem_heladera");
    sem_unlink("sem_platos");

    // ============ RECURSOS COMPARTIDOS (MESADA, HELADERA, PLATOS DEL DÍA) ============

    int SIZE = 4096;  // Tamaño de la memoria compartida 4096 bytes.

    // ============ RECURSO COMPARTIDO (HELADERA) ============
    int shm_fd_heladera;  // Descriptor de archivo de memoria compartida.
    int *heladera;        // Puntero para acceder a la memoria compartida (heladera).
    // ========= RECURSO COMPARTIDO (PLATOS DEL DIA) =========
    int shm_fd_platos;    // Descriptor de archivo de memoria compartida.
    int *platos_del_dia;  // Puntero para acceder a la memoria compartida (heladera).

    // Abrimos las regiones de memoria compartida.
    shm_fd_heladera = shm_open("heladera", O_CREAT | O_RDWR, 0666);
    shm_fd_platos = shm_open("platos", O_CREAT | O_RDWR, 0666);

    // Configuramos el tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd_heladera, 4096);
    ftruncate(shm_fd_platos, 4096);

    // Mapeamos la región de memoria a un puntero para poder acceder a ella.
    heladera = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_heladera, 0);
    *heladera = 25;  // La cantina abre con la heladera llena.

    platos_del_dia = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_platos, 0);
    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día.

    // ========================================================

    // ==== Semáforos HELADERA, REPOSTERO Y PLATOS DEL DIA ====
    sem_t *sem_heladera, *sem_repostero, *sem_platos;
    // El valor del semáforo de la heladera representa la cantidad de espacios libres en la heladera.
    sem_heladera = sem_open("sem_heladera", O_CREAT, 0644, 0);    // sem heladera contador inicilizado en 0 (la cantina abre con la heladera llena. 0 espacios libres).
    sem_repostero = sem_open("sem_repostero", O_CREAT, 0644, 1);  // sem repostero binario inicilizado en 1.
    sem_platos = sem_open("sem_platos", O_CREAT, 0644, 1);        // sem platos del día binario inicializado en 1.

    // ========================================================

    // =============== Codigo del repostero ===================
    // TODO: CICLO DE LA CANTINA 12 MEDIODIA.
    // Tomamos el valor actual del semáforo de la heladera.
    int valor_heladera;
    sem_getvalue(sem_heladera, &valor_heladera);

    // ¿Está vacía la heladera?
    if (valor_heladera == 0) {
        // Está llena (0 espacios libres).
        printf("Heladera llena\n");
    } else {
        // Está vacía (25 espacios libres).
        // Toma el valor actual del sem_heladera.
        sem_getvalue(sem_heladera, &valor_heladera);

        // Procede a llenar la heladera.
        while (valor_heladera != 0) {
            preparar_flan();  // prepara el flan (espera un tiempo aleatorio).

            // Secciones críticas.
            sem_wait(sem_repostero);
            *heladera = *heladera + 1;
            printf("Quedan %d espacios libres en la heladera\n", valor_heladera);
            sem_wait(sem_heladera);  // 1 espacio menos en la heladera.
            sem_post(sem_repostero);

            sem_wait(sem_platos);
            *platos_del_dia = *platos_del_dia + 1;
            printf("Plato numero %d del día\n", *platos_del_dia);
            sem_post(sem_platos);

            sem_getvalue(sem_heladera, &valor_heladera);
        }

        if (valor_heladera == 0) {
            printf("Heladera llena\n");
        }
    }

    // ================ FINAL DEL PROGRAMA ====================
    // Cerramos el descriptor de archivo de memoria compartida.
    close(shm_fd_heladera);
    close(shm_fd_platos);

    // Eliminamos la región de memoria compartida.
    shm_unlink("heladera");
    shm_unlink("platos");

    // Cerramos los semáforos.
    sem_close(sem_heladera);
    sem_unlink("sem_heladera");

    sem_close(sem_repostero);
    sem_unlink("sem_repostero");

    sem_close(sem_platos);
    sem_unlink("sem_platos");
    // ========================================================

    return 0;
}
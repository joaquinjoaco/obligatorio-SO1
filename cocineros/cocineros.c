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

void preparar_plato() {
    // Prepara un flan (dummy).
    printf("Preparando plato...\n");
    // srand(time(NULL));
    // int randomSleep = rand() % 10;
    // sleep(randomSleep);
    sleep(1);
    printf("Plato listo.\n");
}

void cerrar_cantina() {
    // Cierra la cantina (dummy).
    printf("Ordenando cantina...\n");
    sleep(1);
    printf("Recogiendo todos los platos...\n");
    sleep(1);
    printf("Lavando todos los platos...\n");
    sleep(1);
    printf("Guardando utensilios...\n");
    sleep(1);
    printf("Cerrando el local...\n");
    sleep(1);
}

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
    shm_fd_mesada = shm_open("mesada", O_CREAT | O_RDWR, 0666);
    shm_fd_platos = shm_open("platos", O_CREAT | O_RDWR, 0666);

    // Configuramos los tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd_mesada, 4096);
    ftruncate(shm_fd_platos, 4096);

    // Mapeamos las regiones de memoria a los punteros para poder acceder a ellas.
    mesada = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_mesada, 0);
    *mesada = 0;  // La cantina abre con la mesada vacía.

    platos_del_dia = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_platos, 0);
    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día en 0.

    // ========================================================
    // ===== Semáforos COCINERO, MESADA Y PLATOS DEL DIA =====
    sem_t *sem_cocinero, *sem_mesada, *sem_platos;
    // El valor del semáforo de la heladera representa la cantidad de espacios libres en la heladera.
    sem_cocinero = sem_open("sem_cocinero", O_CREAT, 0644, 1);  // sem cocinero inicilizado en 0.
    // El valor del semáforo de la mesada representa la cantidad de espacios libres en ella.
    sem_mesada = sem_open("sem_mesada", O_CREAT, 0644, 27);  // sem mesada contador inicilizado en 27 (la cantina abre con la mesada vacía. 27 espacios libres)
    sem_platos = sem_open("sem_platos", O_CREAT, 0644, 1);   // sem platos del día inicializado en 1.
    // ========================================================

    // ================ Código del cocinero ===================
    for (int i = 0; i < 3; i++)
    // SE CREAN 3 COCINEROS.
    {
        if (fork() == 0) {  // desde el hijo (cocinero)
            printf("COCINERO(%d) pid: %d | ppid: %d \n", i, getpid(), getppid());

            // Tomamos el valor actual del semáforo de la mesada.
            int valor_mesada;
            sem_getvalue(sem_mesada, &valor_mesada);

            // Loop infinito necesario para poder hacer funcionar la cantina varios días.
            // Cada iteración es un día distinto.
            while (1) {
                // Mientras no se hayan cocinado todos los platos del día el cocinero trabaja.
                while (*platos_del_dia < 180) {
                    // ¿Está vacía la mesada?
                    // if (valor_mesada == 0) {
                    if (*mesada == 27) {
                        // Está llena (0 espacios libres).
                        printf("Mesada llena, esperando...\n");
                        sleep(1);
                    } else {
                        // Está vacía (27 espacios libres).
                        // Toma el valor actual del sem_mesada.
                        sem_getvalue(sem_mesada, &valor_mesada);

                        // Procede a llenar la mesada.
                        // while (valor_mesada != 0) {
                        while (*mesada <= 27) {
                            if (*platos_del_dia == 180) {
                                printf("se hicieron los 180 platos");
                                // sem_wait(sem_cocinero);

                            } else {
                                // prepara el plato, función dummy (espera un tiempo aleatorio).
                                preparar_plato();
                                // Secciones críticas.
                                sem_wait(sem_cocinero);
                                *mesada = *mesada + 1;
                                printf("Quedan %d espacios libres en la mesada\n", valor_mesada);
                                sem_wait(sem_mesada);  // 1 espacio menos en la mesada.
                                sem_post(sem_cocinero);

                                // actualiza los platos del día.
                                sem_wait(sem_platos);
                                *platos_del_dia = *platos_del_dia + 1;
                                printf("Plato numero %d del día\n", *platos_del_dia);
                                sem_post(sem_platos);

                                sem_getvalue(sem_mesada, &valor_mesada);
                            }
                        }

                        sem_getvalue(sem_mesada, &valor_mesada);
                        // if (valor_mesada == 0) {
                        if (*mesada == 27) {
                            printf("Mesada llena\n");
                        }
                    }
                }

                // Si ya se hicieron los 180 platos del día
                // se cierra la cantina y se vuelve a abrir.
                if (*platos_del_dia == 180) {
                    printf("se hicieron los 180 platos.\n");
                    sem_wait(sem_cocinero);

                    // Se cierra la cantina.
                    cerrar_cantina();
                    // Se abre la cantina nuevamente. Se reinician todos los valores.
                    printf("Abriendo el local...\n");

                    sem_close(sem_mesada);
                    sem_unlink("sem_mesada");
                    sem_mesada = sem_open("sem_mesada", O_CREAT, 0644, 27);  // sem mesada contador inicilizado en 27 (la cantina abre con la mesada vacía. 27 espacios libres).

                    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día en 0.
                    *mesada = 0;          // La cantina abre con la mesada vacía.

                    printf("¡Local abierto!\n");
                    sem_post(sem_cocinero);
                }
            }

            exit(0);
        }
    }

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
// --------------------------------------------
// Obligatorio Sistemas Operativos
// Tecnologo Informatico 2023
//
// mozos.c
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

void tomar_pedido(int tipo_pedido, int cantidad) {
    // TOMA UN PEDIDO.
    printf("Tomando pedido...\n");

    srand(time(NULL));
    tipo_pedido = rand() % 2;
    if (tipo_pedido == 0) {
        // pidieron flanes.
        cantidad = rand() % (6 + 1 - 1) + 1;  // máximo de 6 flanes, mínimo de 1.
        printf("%d Flanes en camino.\n", cantidad);
    } else {
        // pidieron albóndigas.
        cantidad = rand() % (4 + 1 - 1) + 1;  // máximo de 4 flanes, mínimo de  1.
        printf("%d Platos de albóndiga en camino.\n", cantidad);
    }

    sleep(1);
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
    sem_unlink("sem_heladera");
    sem_unlink("sem_mesada");
    sem_unlink("sem_mozo");
    sem_unlink("sem_platos");

    // ============ RECURSOS COMPARTIDOS (MESADA, HELADERA,PLATOS DEL DÍA) ============

    int SIZE = 4096;  // Tamaño de la memoria compartida 4096 bytes.

    // ============ RECURSO COMPARTIDO (MESADA) ============
    int shm_fd_mesada;  // Descriptor de archivo de memoria compartida.
    int *mesada;        // Puntero para acceder a la memoria compartida.
    // =========== RECURSO COMPARTIDO (HELADERA) ===========
    int shm_fd_heladera;  // Descriptor de archivo de memoria compartida.
    int *heladera;        // Puntero para acceder a la memoria compartida.
    // ========= RECURSO COMPARTIDO (PLATOS DEL DIA) =========
    int shm_fd_platos;    // Descriptor de archivo de memoria compartida.
    int *platos_del_dia;  // Puntero para acceder a la memoria compartida (heladera).

    // Abrimos las regiones de memoria compartida.
    shm_fd_mesada = shm_open("mesada", O_CREAT | O_RDWR, 0666);
    shm_fd_heladera = shm_open("heladera", O_CREAT | O_RDWR, 0666);
    shm_fd_platos = shm_open("platos", O_CREAT | O_RDWR, 0666);

    // Configuramos los tamaño de la memoria compartida 4096 bytes.
    ftruncate(shm_fd_mesada, 4096);
    ftruncate(shm_fd_heladera, 4096);
    ftruncate(shm_fd_platos, 4096);

    // Mapeamos las regiones de memoria a los punteros para poder acceder a ellas.
    mesada = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_mesada, 0);
    *mesada = 0;  // La cantina abre con la mesada vacía.

    heladera = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_heladera, 0);
    *heladera = 25;  // La cantina abre con la heladera llena.

    platos_del_dia = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_platos, 0);
    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día en 0.
    // ========================================================

    // ====== Semáforos  HELADERA, MOZO Y PLATOS DEL DIA ======
    sem_t *sem_heladera, *sem_mozo, *sem_platos, *sem_mesada, *sem_repostero;
    // El valor del semáforo de la heladera representa la cantidad de espacios libres en la heladera.
    sem_heladera = sem_open("sem_heladera", O_CREAT, 0644, 0);  // sem heladera contador inicilizado en 0 (la cantina abre con la heladera llena. 0 espacios libres).
    sem_mozo = sem_open("sem_mozo", O_CREAT, 0644, 1);          // sem mozo binario inicilizado en 1.
    sem_platos = sem_open("sem_platos", O_CREAT, 0644, 1);      // sem platos del día binario inicializado en 1.
    // El valor del semáforo de la mesada representa la cantidad de espacios libres en ella.
    sem_mesada = sem_open("sem_mesada", O_CREAT, 0644, 27);       // sem mesada contador inicilizado en 27 (la cantina abre con la mesada vacía. 27 espacios libres).
    sem_repostero = sem_open("sem_repostero", O_CREAT, 0644, 1);  // sem repostero binario inicilizado en 1.

    // ========================================================

    // ================== Código del mozo =====================
    for (int i = 0; i < 5; i++)
    // SE CREAN 5 MOZOS.
    {
        if (fork() == 0) {  // desde el hijo (mozo)
            printf("MOZO(%d) pid: %d | ppid: %d \n", i, getpid(), getppid());

            int valor_mesada;
            int valor_heladera;
            // Toma el valor actual del sem_mesada.
            sem_getvalue(sem_mesada, &valor_mesada);

            // Loop infinito necesario para poder hacer funcionar la cantina varios días.
            // Cada iteración es un día distinto.
            while (1) {
                // Mientras que haya espacio en la mesada y no se hayan servido los 180 platos del día el mozo trabaja.
                while (valor_mesada != 0 || *platos_del_dia < 180) {
                    int tipo_pedido;  // 0 FLANES, 1 ALBÓNDIGAS.
                    int cantidad;     // 4 maximo de albóndigas, 6 máximo de flanes.

                    // Toma un pedido en cada iteración.
                    tomar_pedido(tipo_pedido, cantidad);

                    if (tipo_pedido == 0) {
                        // Pidieron flan. Se fija si la heladera está vacía.

                        // Toma el valor actual del sem_heladera.
                        sem_getvalue(sem_heladera, &valor_heladera);

                        // if (valor_heladera == 25) {
                        if (*heladera == 0) {
                            // la heladera está vacía.
                            // le avisa al repostero que llene la heladera.
                            sem_post(sem_repostero);

                            // } else if (valor_heladera < 19) {
                        } else if (*heladera >= 6) {
                            // Hay al menos 6 flanes en la heladera (19 espacios libres).
                            // Retira flanes hasta que haya conseguido la cantidad encargada.

                            sem_wait(sem_mozo);
                            while (cantidad > 0) {
                                *heladera = *heladera - 1;
                                printf("Quedan %d flanes en la heladera\n", *heladera);
                                sem_post(sem_heladera);  // un espacio libre mas a la heladera.
                                // Toma de nuevo el valor de la heladera para la próxima iteración.
                                sem_getvalue(sem_heladera, &valor_heladera);
                            }
                            sem_post(sem_mozo);
                        } else {
                            printf("No hay suficientes flanes.\n");
                            sleep(1);  // espera un segundo a que halla suficiente espacio.
                        }

                    } else {
                        // Pidieron albóndigas.
                        // Toma el valor actual del sem_mesada.
                        sem_getvalue(sem_mesada, &valor_mesada);

                        // if (valor_mesada < 23) {
                        if (*mesada >= 4) {
                            // Hay al menos 4 albondigas en la mesada (23 espacios libres).
                            sem_wait(sem_mozo);
                            while (cantidad > 0) {
                                *mesada = *mesada - 1;
                                printf("Quedan %d platos servidos en la mesada\n", *mesada);
                                sem_post(sem_mesada);  // un espacio libre mas a la mesada.

                                // Toma de nuevo el valor de la mesada para la próxima iteración.
                                sem_getvalue(sem_mesada, &valor_mesada);
                            }
                            sem_post(sem_mozo);
                        } else {
                            printf("No hay suficientes platos de albóndiga.\n");
                            sleep(1);  // espera.
                        }
                    }
                }

                // Si ya se hicieron los 180 platos del día
                // se cierra la cantina y se vuelve a abrir.
                if (*platos_del_dia == 180) {
                    printf("se hicieron los 180 platos.\n");
                    sem_wait(sem_mozo);

                    // Se cierra la cantina.
                    cerrar_cantina();
                    // Se abre la cantina nuevamente. Se reinician todos los valores.
                    printf("Abriendo el local...\n");

                    shm_unlink("heladera");
                    sem_close(sem_heladera);
                    sem_heladera = sem_open("sem_heladera", O_CREAT, 0644, 0);

                    sem_close(sem_mesada);
                    sem_unlink("sem_mesada");
                    sem_mesada = sem_open("sem_mesada", O_CREAT, 0644, 27);  // sem mesada contador inicilizado en 27 (la cantina abre con la mesada vacía. 27 espacios libres).

                    *heladera = 25;       // La cantina abre con la heladera llena.
                    *platos_del_dia = 0;  // La cantina abre con la cantidad de platos del día en 0.
                    *mesada = 0;          // La cantina abre con la mesada vacía.

                    printf("¡Local abierto!\n");
                    sem_post(sem_mozo);
                }
            }

            exit(0);
        }
    }

    // ================ FINAL DEL PROGRAMA ====================
    // Cerramos los descriptores de archivo de memoria compartida.
    close(shm_fd_mesada);
    close(shm_fd_heladera);
    close(shm_fd_platos);

    // Eliminamos las regiones de memoria compartida.
    shm_unlink("heladera");
    shm_unlink("mesada");
    shm_unlink("platos");

    // Cerramos los semáforos.
    sem_close(sem_heladera);
    sem_unlink("sem_heladera");

    sem_close(sem_mozo);
    sem_unlink("sem_mozo");

    sem_close(sem_platos);
    sem_unlink("sem_platos");

    sem_close(sem_repostero);
    sem_unlink("sem_repostero");

    sem_close(sem_mesada);
    sem_unlink("sem_mesada");
    // ========================================================

    return 0;
}
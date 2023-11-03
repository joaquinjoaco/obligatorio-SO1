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

// Un restaurante sirve solo un plato, sopa de cebolla. En el restaurante
// trabajan 2 cocineros y 3 mozos. Entre el salón comedor y la cocina hay
// un mostrador, donde los cocineros van colocando los platos de sopa a
// medida que los van sirviendo y de donde los mozos retiran dichos platos
// para llevarlos a las mesas. En el mostrador caben exactamente 12
// platos. Modelar el comportamiento de los cocineros y de los mozos,
// considerando que el mostrador es compartido entre ambos.

// Sugerencias:
// • Identificar cual es el recurso compartido.
// • Identificar las posibles condiciones de error en el acceso a dicho
// recurso.
// • Identificar los procesos correspondientes a los distintos actores.

// gcc -o main main.c -lpthread -lrt

// cocinero
void coloca_plato()
{
    printf("cocinero coloca plato pid: %d | ppid: %d \n", getpid(), getppid());
    sleep(1);
    printf("cocinero termina de colocar plato pid: %d | ppid: %d \n", getpid(), getppid());
}

// mozo
void levanta_pedido()
{
    printf("mozo levanta pedido pid: %d | ppid: %d \n", getpid(), getppid());
    sleep(1);
    printf("mozo terminó levanta pedido pid: %d | ppid: %d \n", getpid(), getppid());
}
void entrega_pedido()
{
    printf("mozo entrega pedido pid: %d | ppid: %d \n", getpid(), getppid());
    sleep(1);
    printf("mozo terminó entrega pedido pid: %d | ppid: %d \n", getpid(), getppid());
}

int main()
{
    // ---------- Shared memory (mostrador) ----------
    int shm_fd;
    int *mostrador = 0;
    // crear memoria compartida
    shm_fd = shm_open("memoriacompartida", O_CREAT | O_RDWR, 0666);
    // configurar el tamaño de la memoria compartida 4096 bytes
    int SIZE = 4096;
    ftruncate(shm_fd, 4096);

    mostrador = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    *mostrador = 0;
    // -----------------------------------------------

    // ---------- Semaphore sc & m & c ----------
    sem_t *sem_sc, *sem_m, *sem_c;
    sem_sc = sem_open("sem", O_CREAT, 0644, 1);      // sem shm inicilizado en 1
    sem_m = sem_open("mozo", O_CREAT, 0644, 0);      // sem mozo inicilizado en 1
    sem_c = sem_open("cocinero", O_CREAT, 0644, 12); // sem cocinero inicilizado en 12
    // -------------------------------------

    // 2 cocineros y 3 mozos, creamos 5 procesos

    for (int i = 0; i < 2; i++)
    // creamos 2 cocineros
    {
        int q = fork();
        if (q == 0)
        { // estamos en el hijo (cocinero)
            printf("cocinero pid: %d | ppid: %d \n", getpid(), getppid());
            while (1)
            {
                sem_wait(sem_c); // -1
                printf("%d", *mostrador);
                coloca_plato();
                // mostrador memoria compartida
                sem_wait(sem_sc); // -1
                *mostrador++;
                sem_post(sem_sc); // +1

                sem_post(sem_m); // +1
            }
            exit(-1);
        }
        if (q > 0)
        {
            // padre (no hacer nada)
        }
    }

    for (int i = 0; i < 3; i++)
    // creamos 3 mozos
    {
        int p = fork();
        if (p == 0)
        { // estamos en el hijo (mozo)
            printf("mozo pid: %d | ppid: %d \n", getpid(), getppid());
            while (1)
            {
                // Hay platos en el mostrador
                sem_wait(sem_m); // -1
                printf("%d", *mostrador);
                levanta_pedido();
                // mostrador memoria compartida
                sem_wait(sem_sc); // -1
                *mostrador--;
                sem_post(sem_sc); // +1
                entrega_pedido();

                sem_post(sem_c); // +1
            }
            exit(-1);
        }
        if (p > 0)
        {
            // padre (no hacer nada)
        }
    }

    return 0;
}
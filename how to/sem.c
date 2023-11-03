#include <semaphore.h>
#include <sys/ipc.h>
#include <fcntl.h>

main() {
    sem_t *sem;
    sem = sem_open("sem.txt", O_CREAT, 0644,1);
    // --------------------
    sem_wait(sem);
    // sección crítica
    sem_post(sem);
    // --------------------
    sem_close(sem);
    sem_unlink("sem.txt");
}
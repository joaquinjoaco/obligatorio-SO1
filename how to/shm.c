#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


int main() {
    int shm_fd;
    int *ptr;


    // crear memoria compartida
    shm_fd = shm_open("nombre", O_CREAT | O_RDWR, 0666);
    // Configurar el tamaño de la memoria compartida 4096 bytes
    ftruncate(shm_fd, 4096);
    int SIZE = 4096;
    ptr=(int*)mmap(0,SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    // Usar la memoria compartida
    *ptr=4;
    // --------------------------
    close(shm_fd);
    shm_unlink("nombre");

}
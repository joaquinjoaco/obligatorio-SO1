#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
	int x = fork();

	if ( x > 0 ) { // estamos en el parent
		printf("X pid: %d | ppid: %d \n", getpid(), getppid());
		int y = fork(); // hacemos otro fork desde el parent
		
		if ( y > 0 ) { // estamos en el parent
			printf("Y pid: %d | ppid: %d \n", getpid(), getppid());
			int z = fork(); // hacemos otro fork desde el parent
		
			if ( z > 0 ) { // estamos en el parent 
				printf("Z pid: %d | ppid: %d \n", getpid(), getppid());
			}	
		}
	}
	
	return 0;
}

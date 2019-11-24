#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/sem.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#define SHMSIZE 1024 
#define SHMKEY (key_t)0111
#define SEMKEY (key_t)0111

int CopyRead(); 
int CopyWrite();

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
};
static int  semid;

int main(int argc, char **argv) {
	FILE* fp;
	union semun sem_union;

	struct sembuf mysem_open  = {0, -1, SEM_UNDO};
    	struct sembuf mysem_close = {0, 1, SEM_UNDO};
	int sem_num;

	if (argc > 1)
        	sem_num = 1;
    	else 
        	sem_num = 0;

	semid = semget((key_t)234, sem_num, 0660|IPC_CREAT);
    	if (semid == -1) {
        	perror("semget error ");
        	exit(0);
    	}

	sem_union.val = 1;
        if ( -1 == semctl( semid, 0, SETVAL, sem_union)) {   
                printf( "semctl()-SETVAL 실행 오류\n");
                return -1; 
        }

	if(semop(semid, &mysem_open, 1) == -1) {
        	perror("semop error ");
        	exit(0);
    	}

    	if ((fp = fopen("a.txt", "r+")) == NULL) {
        	perror("fopen error ");
        	exit(0);
    	}

	CopyRead();
	rewind(fp);

	sleep(3);
    	CopyWrite();

	fclose(fp);

	semop(semid, &mysem_close, 1);
    	return 1;


}

int CopyRead() { //parents process (file read)
	int shmid, len; 
	void *shmaddr;
	FILE *fp = fopen("a.txt", "r");
	int size = strlen((char *)shmaddr);
	
	printf("read start\n");

	if ((shmid = shmget(SHMKEY, SHMSIZE,IPC_CREAT|0666)) == -1) { 
		perror ("shmget failed"); 
		exit (1); 
	} 

	if ((shmaddr = shmat(shmid, NULL, 0)) == (void *)-1) { 
		perror ("shmat failed"); 
		exit (1); 
	} 

	fgets((char *)shmaddr, size, fp);

	if (shmdt(shmaddr) == -1) { 
		perror ("shmdt failed"); 
		exit (1);
	}

	fclose(fp);
}

int CopyWrite() { //child process (file write)
	int shmid, len; 
	void *shmaddr;
	FILE *fp = fopen("b.txt", "w");

	if ((shmid = shmget(SHMKEY, SHMSIZE,IPC_CREAT|0666)) == -1) { 
		perror ("shmget failed"); 
		exit (1); 
	}

	if ((shmaddr=shmat(shmid, NULL, 0)) == (void *)-1) { 
		perror ("shmat failed"); 
		exit (1); 
	}

	fputs((char *)shmaddr, fp);
	printf("Copy Success!\n");

	if (shmdt(shmaddr) == -1) { 
		perror ("shmdt failed"); 
		exit (1); 
	}

	if (shmctl(shmid, IPC_RMID, 0) == -1) { 
		perror ("shmctl failed"); 
		exit (1); 
	}

	fclose(fp);
}


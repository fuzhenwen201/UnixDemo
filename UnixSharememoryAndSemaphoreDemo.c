#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <string.h>
#include <errno.h>

#define SHM_IPCKEY 11
#define SEM_IPCKEY 22
#define TEST_COUNT 5

typedef struct shared_st
{
	char cContent[1024];
	short uNum;
	int iCount;
}shared_st;

union semun
{
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                    (Linux-specific) */
};


static int set_semvalue(int semid, int semnum, int initval)
{
	union semun sem_union;

	sem_union.val = initval;
	if(semctl(semid, semnum, SETVAL, sem_union) == -1)
	{
		fprintf(stderr, "set_semvalue failed, semctl error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static int del_semvalue(int semid)
{
	union semun sem_union;
	
	if(semctl(semid, 0, IPC_RMID, sem_union) == -1)
	{
		fprintf(stderr, "del_semvalue failed, semctl error: %s\n", strerror(errno));
		return -1;
	}
	return 0;	
}

static int semaphore_p(int semid)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;

	if(semop(semid, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_p failed, semop error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static int semaphore_v(int semid)
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	
	if(semop(semid, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_v failed, semop error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void child_process(const char *programPath)
{
	key_t shm_key = ftok(programPath, SHM_IPCKEY);
	printf("[child] shm_key = 0x%x\n", shm_key);

	//1. attch sharememory
	int shmid = shmget(shm_key, sizeof(shared_st), 0666);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[child] shmid = %d\n", shmid);
	
	//2. attch sharememory
	void *shm = shmat(shmid, 0, 0);
	if(shm == (void *)-1)
	{
		fprintf(stderr, "shmat failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("[child] Memory attached at %p\n", shm);
	
	

	//3. create semaphore
	key_t sem_key = ftok(programPath, SEM_IPCKEY);
	int semid;
	semid = semget(sem_key, 1, 0666);
	if(semid == -1)
	{
		fprintf(stderr, "semget failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int i=0;
	for(; i<TEST_COUNT; i++)
	{
		//4. enter critical zone
		if(semaphore_p(semid) == -1)
		{
			exit(EXIT_FAILURE);
		}
	
		//5. read data
		shared_st *pShared = (shared_st *)shm;

		printf("[child] stShared: cContent: %s, nNum: %d, iCount: %d\n\n", 
			pShared->cContent,
			pShared->uNum,
			pShared->iCount);

		//7. exit critical zone
		if(semaphore_v(semid) == -1)
		{
			exit(EXIT_FAILURE);
		}
	}

	//8. detach sharememory
	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed, error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[child] detach sharememory success!\n");
}

void parent_process(const char *programPath)
{
	key_t shm_key = ftok(programPath, SHM_IPCKEY);
	printf("[parent] shm_key = 0x%x\n", shm_key);		
	
	//1. create sharememory
	int shmid = shmget(shm_key, sizeof(shared_st), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		fprintf(stderr, "shmget failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[parent] shmid = %d\n", shmid);
	
	//2. attch sharememory
	void *shm = shmat(shmid, 0, 0);
	if(shm == (void *)-1)
	{
		fprintf(stderr, "shmat failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("[parent] Memory attached at %p\n", shm);
	
	

	//3. create semaphore
	key_t sem_key = ftok(programPath, SEM_IPCKEY);
	int semid = semget(sem_key, 1, 0666|IPC_CREAT);
	if(semid == -1)
	{
		fprintf(stderr, "semget failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//4. init semaphore
	if(set_semvalue(semid, 0, 1) == -1)
	{
		exit(EXIT_FAILURE);
	}

	int i=0;
	for(; i<TEST_COUNT; i++)
	{
		//5. enter critical zone
		if(semaphore_p(semid) == -1)
		{
			exit(EXIT_FAILURE);
		}
	
		//6. write data
		shared_st *pShared = (shared_st *)shm;
		strncpy(pShared->cContent, "hello", i+1);
		pShared->uNum = i+1;
		pShared->iCount = time(NULL);

		sleep(1);
		printf("[parent] stShared: cContent: %s, uNum: %d, iCount: %d\n", 
			pShared->cContent,
			pShared->uNum,
			pShared->iCount);

		//7. exit critical zone
		if(semaphore_v(semid) == -1)
		{
			exit(EXIT_FAILURE);
		}
	}
	
	sleep(2);

	//8. delete semaphore
	if(del_semvalue(semid) == -1)
	{
		exit(EXIT_FAILURE);
	}
	printf("[parent] delete semaphore success!\n");
	
	//9. detach sharememory
	if(shmdt(shm) == -1)
	{
		fprintf(stderr, "shmdt failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[parent] detach sharememory success!\n");

	//10. delete sharememory
	if(shmctl(shmid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "shmctl failed! error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("[parent] delete sharememory success!\n");

	exit(EXIT_SUCCESS);
	
}


int main(int argc, char **argv)
{
	char programPath[64];
	getcwd(programPath, sizeof(programPath));
	strcat(programPath, "/");

	// programname: argv[0]
	char* ptr = strrchr(argv[0], '/');
	if(ptr == NULL)
	{
		strcat(programPath, argv[0]);
	}
	else
	{
		strcat(programPath, ptr+1);
	}
	
	printf("absolute program path: %s\n", programPath);



	pid_t pid = fork();
	if(pid == 0) //child
	{
		printf("[child] child process create! pid = %d\n", getpid());
		child_process(programPath);
	}
	else if(pid > 0) //parent
	{
		printf("[parent] child process pid = %d\n", pid);
		parent_process(programPath);
	}
	else
	{
		fprintf(stderr, "fork failed, error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 0;
}

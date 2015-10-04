#include "network.h"

int main(int argc, char *argv[])
{
	int len = BLOCK_SIZE;
	int shmid = 0;
	int* addr = NULL;
	struct shmid_ds shmbuf;
	int i = 0;
	key_t key = ftok("/dev/shm/shm-mamo", 'x');
	
	// create share memory
	shmid = shmget(key, len, PERM);
	if (shmid < 0)
	{
		perror("shmget error:");
		return -1;
	}
	
	// attach, get share memory address
	addr = shmat(shmid, NULL, 0);
	if ((int)addr == -1)
	{
		perror("shmat error:");
		return -1;
	}

	// get shmid_ds struct
	shmctl(shmid, IPC_STAT, &shmbuf);

	// write operation in share memory
	for (i = 0; i < shmbuf.shm_segsz / sizeof(int); i++)
	{
		addr[i] = i;
	}
	for (i = 0; i < 10; i++)
	{
		printf("shmget id=%d, addr=%x, size=%d\n", shmid, (unsigned int)addr, shmbuf.shm_segsz);
		sleep(60);
	}

	// delete share memory
	shmctl(shmid, IPC_RMID, NULL);
	
	return 0;
}


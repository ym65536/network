#include "network.h"

int main(int argc, char *argv[])
{
	int len = BLOCK_SIZE;
	int shmid = 0;
	int* addr = NULL;
	struct shmid_ds shmbuf;
	int i = 0;
	key_t key = ftok("/dev/shm/shm-mamo", 'x');
	
	shmid = shmget(key, len, PERM);
	if (shmid < 0)
	{
		perror("shmget error:");
		return -1;
	}
	
	addr = shmat(shmid, NULL, 0);
	if ((int)addr == -1)
	{
		perror("shmat error:");
		return -1;
	}
	shmctl(shmid, IPC_STAT, &shmbuf);

	printf("shmget id=%d, addr=%x, size=%d\n", shmid, (unsigned int)addr, shmbuf.shm_segsz);
	for (i = 0; i < shmbuf.shm_segsz / sizeof(int); i++)
	{
		if ((i & 0x0f) == 0)
		{
			printf("\n");
		}
		printf("%d ", addr[i]);
	}
	
	return 0;
}


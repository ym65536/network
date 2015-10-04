#include "network.h"

struct shared
{
	int count;
	sem_t mutex;
};
	
struct shared share;

int main(int argc, char* argv[])
{
	int fd = -1;
	int nloop = 1000;
	struct shared *addr = NULL;
	int i = 0;

	if (argc == 2)
	{
		nloop = atoi(argv[1]);
	}

	fd = open("/tmp/mamoshare", O_RDWR | O_CREAT, FILE_MODE);
	if (fd < 0)
	{
		perror("open error");
		return -1;
	}
	write(fd, &share, sizeof(share));
	addr = mmap(NULL, sizeof(share), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!addr)
	{
		perror("mmap error");
		return -1;
	}

	sem_init(&addr->mutex, 1, 1);
	if (fork() == 0) // child
	{
		for (i = 0; i < nloop; i++)
		{
			sem_wait(&addr->mutex);
			printf("child: %d\n", addr->count++);
			sem_post(&addr->mutex);
		}
		exit(0);
	}

	for (i = 0; i < nloop; i++)
	{
		sem_wait(&addr->mutex);
		printf("parent: %d\n", addr->count++);
		sem_post(&addr->mutex);
	}
	sleep(1);

	return 0;
}

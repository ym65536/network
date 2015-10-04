#ifndef __NETWORK_H
#define __NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#define PERM 		(0666 | IPC_CREAT)
#define BLOCK_SIZE	1024

#define FILE_MODE	(0666)

#endif


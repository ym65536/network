#include "common.h"

int main(void)
{
	if (st_init() < 0)
	{
		printf("state thread init fail!\n");
		return -1;
	}



	return 0;
}

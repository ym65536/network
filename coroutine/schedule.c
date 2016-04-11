#include "common.h"

st_thread_t st_thread_pool[MAX_COROUTINE];  //indexed by accepted socket fd

int accepted_sock[MAX_EVENTS]; //accepted socket id indexed by event fd

void Schedule(int conn_fd)
{
	st_thread_t *pctx = &st_thread_pool[conn_fd];

	//last saved env, we will jump there
	jmp_buf last_env;
	memcpy(last_env, pctx->context, sizeof(jmp_buf));

	//save current env and jump to last save point (inside epoll loop)
	if (MT_SETJMP(pctx->context) == 0)
	{
		printf("[UserRoutine %d] @Schedule: Save and Switch st_thread\n", conn_fd); 		
		MT_LONGJMP(last_env, 1);
	}

	//re-schedule jumps back here
	printf("[UserRoutine %d] @Schedule: st_thread Resumed\n", conn_fd);
}


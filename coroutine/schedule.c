#include "common.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define REDZONE	_ST_PAGE_SIZE

/* Global data */
_st_vp_t _st_this_vp;           /* This VP */
st_thread_t *_st_this_thread;  /* Current thread */
int _st_active_count = 0;       /* Active thread count */

static char *_st_new_stk_segment(int size)
{
	static int zero_fd = -1;
	int mmap_flags = MAP_PRIVATE;
	void *vaddr;

	vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, zero_fd, 0);
	if (vaddr == (void *)MAP_FAILED)
		return NULL;

	return (char *)vaddr;
}

st_stack_t *_st_stack_new(int stack_size)
{
  st_stack_t *ts;

  /* Make a new thread stack object. */
  if ((ts = (st_stack_t *)calloc(1, sizeof(st_stack_t))) == NULL)
    return NULL;
  ts->vaddr_size = stack_size + 2 * REDZONE;
  ts->vaddr = _st_new_stk_segment(ts->vaddr_size);
  if (!ts->vaddr) 
  {
    free(ts);
    return NULL;
  }
  ts->stk_size = stack_size;
  ts->stk_bottom = ts->vaddr + REDZONE;
  ts->stk_top = ts->stk_bottom + stack_size;

  return ts;
}

void _st_thread_main(void)
{
	st_thread_t *thread = _ST_CURRENT_THREAD();

    /* Run thread main */
	thread->retval = (*thread->start)(thread->arg);

  /* All done, time to go away */
//	st_thread_exit(thread->retval);
}

st_thread_t *st_thread_create(void *(*start)(void *arg), void *arg)
{
	st_thread_t *thread;
	st_stack_t *stack;
	void **ptds;
	char *sp;

	int stk_size = ST_DEFAULT_STACK_SIZE;
	stk_size = ((stk_size + _ST_PAGE_SIZE - 1) / _ST_PAGE_SIZE) * _ST_PAGE_SIZE;
	stack = _st_stack_new(stk_size);
	if (!stack)
	{
		return NULL;
	}

  /* Allocate thread object and per-thread data off the stack */
	sp = stack->stk_top;
	sp = sp - sizeof(st_thread_t);
	thread = (st_thread_t *) sp;

	/* Make stack 64-byte aligned */
	if ((unsigned long)sp & 0x3f)
	{
		sp = sp - ((unsigned long)sp & 0x3f);
	}
	stack->sp = sp - _ST_STACK_PAD_SIZE;

	memset(thread, 0, sizeof(st_thread_t));

	/* Initialize thread */
	thread->stack = stack;
	thread->start = start;
	thread->arg = arg;

	_ST_INIT_CONTEXT(thread, stack->sp, _st_thread_main);

	/* Make thread runnable */
	thread->state = _ST_ST_RUNNABLE;
	_st_active_count++;
	_ST_ADD_RUNQ(thread);

	return thread;
}

int main(void)
{
	return 0;
}

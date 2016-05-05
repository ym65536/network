#include "common.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>

#define REDZONE	_ST_PAGE_SIZE

/* Global data */
_st_vp_t _st_this_vp;           /* This VP */
st_thread_t *_st_this_thread;  /* Current thread */
int _st_active_count = 0;       /* Active thread count */

st_stack_t *_st_stack_new(int stack_size)
{
	st_stack_t *ts;

  /* Make a new thread stack object. */
	if ((ts = (st_stack_t *)calloc(1, sizeof(st_stack_t))) == NULL)
    	return NULL;
	ts->vaddr_size = stack_size + 2 * REDZONE;
#if 0
  	ts->vaddr = mmap(NULL, ts->vaddr_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
	ts->vaddr = malloc(ts->vaddr_size);
#endif
  	if (ts->vaddr == MAP_FAILED) 
  	{
		perror("mmap");
		return NULL;
  	}
  	ts->stk_size = stack_size;
  	ts->stk_bottom = ts->vaddr + REDZONE;
  	ts->stk_top = ts->stk_bottom + stack_size;

  	return ts;
}

void st_thread_yield(void *retval)
{
  st_thread_t *thread = _ST_CURRENT_THREAD();

  _st_active_count--;

  /* Find another thread to run */
  _ST_SWITCH_CONTEXT(thread);
  /* Not going to land here */
}

void _st_thread_main(void)
{
	st_thread_t *thread = _ST_CURRENT_THREAD();

    /* Run thread main */
	thread->retval = (*thread->start)(thread->arg);

  /* All done, time to go away */
	st_thread_yield(thread->retval);
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

/*
 * Start function for the idle thread
 */
/* ARGSUSED */
void *_st_idle_thread_start(void *arg)
{
  st_thread_t *me = _ST_CURRENT_THREAD();

  while (_st_active_count > 0) 
  {
    /* Idle vp till I/O is ready or the smallest timeout expired */
    //_ST_VP_IDLE();
	printf("Now In Idle thread...\n");

    me->state = _ST_ST_RUNNABLE;
    _ST_SWITCH_CONTEXT(me);
  }

  /* No more threads */
  exit(0);

  /* NOTREACHED */
  return NULL;
}

void _st_vp_schedule(void)
{
  st_thread_t *thread;

  if (_ST_RUNQ.next != &_ST_RUNQ) 
  {
    /* Pull thread off of the run queue */
    thread = _ST_THREAD_PTR(_ST_RUNQ.next);
    _ST_DEL_RUNQ(thread);
  } 
  else 
  {
    /* If there are no threads to run, switch to the idle thread */
    thread = _st_this_vp.idle_thread;
  }
  assert(thread->state == _ST_ST_RUNNABLE);

  /* Resume the thread */
  thread->state = _ST_ST_RUNNING;
  _ST_RESTORE_CONTEXT(thread);
}

/*
 * Initialize this Virtual Processor
 */
int st_init(void)
{
  st_thread_t *thread;

  if (_st_active_count) {
    /* Already initialized */
    return 0;
  }

  memset(&_st_this_vp, 0, sizeof(_st_vp_t));

  ST_INIT_CLIST(&_ST_RUNQ);

  _st_this_vp.pagesize = getpagesize();

  /*
   * Create idle thread
   */
  _st_this_vp.idle_thread = st_thread_create(_st_idle_thread_start, NULL);
  if (!_st_this_vp.idle_thread)
    return -1;
  _st_this_vp.idle_thread->flags = _ST_FL_IDLE_THREAD;
  _st_active_count--;
  _ST_DEL_RUNQ(_st_this_vp.idle_thread);

  /*
   * Initialize primordial thread
   */
  thread = (st_thread_t *) calloc(1, sizeof(st_thread_t));
  if (!thread)
    return -1;
  thread->state = _ST_ST_RUNNING;
  thread->flags = _ST_FL_PRIMORDIAL;
  _ST_SET_CURRENT_THREAD(thread);
  _st_active_count++;

  return 0;
}


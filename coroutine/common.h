#ifndef __ST_COMMON_H__
#define __ST_COMMON_H__

#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>
#include <setjmp.h>
#include "md.h"

void _st_vp_schedule(void);

#define MAX_COROUTINE 1024
#define MAX_EVENTS MAX_COROUTINE
#define MAX_DATA_BUF 256
#define StackProtect char stack_down[1024 * 1024];

#define ST_BEGIN_MACRO {
#define ST_END_MACRO }

/*****************************************
 * Circular linked list definitions
 */

typedef struct _st_clist 
{
  struct _st_clist *next;
  struct _st_clist *prev;
} _st_clist_t;

/* Insert element "_e" into the list, before "_l" */
#define ST_INSERT_BEFORE(_e,_l)	 \
    ST_BEGIN_MACRO		 \
	(_e)->next = (_l);	 \
	(_e)->prev = (_l)->prev; \
	(_l)->prev->next = (_e); \
	(_l)->prev = (_e);	 \
    ST_END_MACRO

/* Insert element "_e" into the list, after "_l" */
#define ST_INSERT_AFTER(_e,_l)	 \
    ST_BEGIN_MACRO		 \
	(_e)->next = (_l)->next; \
	(_e)->prev = (_l);	 \
	(_l)->next->prev = (_e); \
	(_l)->next = (_e);	 \
    ST_END_MACRO

/* Return the element following element "_e" */
#define ST_NEXT_LINK(_e)  ((_e)->next)

/* Append an element "_e" to the end of the list "_l" */
#define ST_APPEND_LINK(_e,_l) ST_INSERT_BEFORE(_e,_l)

/* Insert an element "_e" at the head of the list "_l" */
#define ST_INSERT_LINK(_e,_l) ST_INSERT_AFTER(_e,_l)

/* Return the head/tail of the list */
#define ST_LIST_HEAD(_l) (_l)->next
#define ST_LIST_TAIL(_l) (_l)->prev

/* Remove the element "_e" from it's circular list */
#define ST_REMOVE_LINK(_e)	       \
    ST_BEGIN_MACRO		       \
	(_e)->prev->next = (_e)->next; \
	(_e)->next->prev = (_e)->prev; \
    ST_END_MACRO

/* Return non-zero if the given circular list "_l" is empty, */
/* zero if the circular list is not empty */
#define ST_CLIST_IS_EMPTY(_l) \
    ((_l)->next == (_l))

/* Initialize a circular list */
#define ST_INIT_CLIST(_l)  \
    ST_BEGIN_MACRO	   \
	(_l)->next = (_l); \
	(_l)->prev = (_l); \
    ST_END_MACRO

#define ST_INIT_STATIC_CLIST(_l) \
    {(_l), (_l)}


/*****************************************
 * Basic types definitions
 */
typedef struct st_stack
{
  _st_clist_t links;
  char *vaddr;                /* Base of stack's allocated memory */
  int  vaddr_size;            /* Size of stack's allocated memory */
  int  stk_size;              /* Size of usable portion of the stack */
  char *stk_bottom;           /* Lowest address of stack's usable portion */
  char *stk_top;              /* Highest address of stack's usable portion */
  void *sp;                   /* Stack pointer from C's point of view */
} st_stack_t;


typedef struct st_thread st_thread_t;

struct st_thread 
{
	int state;                  /* Thread's state */
	int flags;                  /* Thread's flags */
	void *(*start)(void *arg);  /* The start function of the thread */
	void *arg;                  /* Argument of the start function */
	void *retval;               /* Return value of the start function */
	st_stack_t *stack;	      /* Info about thread's stack */
	_st_clist_t links;          /* For putting on run/sleep/zombie queue */
	int started;
	char data[MAX_DATA_BUF];
	int data_len;
	int event_fd;
	jmp_buf context; //last saved stack and register info
};

typedef struct _st_eventsys_ops {
  const char *name;                          /* Name of this event system */
  int  val;                                  /* Type of this event system */
  int  (*init)(void);                        /* Initialization */
  void (*dispatch)(void);                    /* Dispatch function */
  int  (*pollset_add)(void *, int); /* Add descriptor set */
  void (*pollset_del)(void *, int); /* Delete descriptor set */
  int  (*fd_new)(int);                       /* New descriptor allocated */
  int  (*fd_close)(int);                     /* Descriptor closed */
  int  (*fd_getlimit)(void);                 /* Descriptor hard limit */
} _st_eventsys_t;


typedef struct _st_vp {
  st_thread_t *idle_thread;  /* Idle thread for this vp */
  //st_utime_t last_clock;      /* The last time we went into vp_check_clock() */

  _st_clist_t run_q;          /* run queue for this vp */
  _st_clist_t io_q;           /* io queue for this vp */
  _st_clist_t zombie_q;       /* zombie queue for this vp */
  int pagesize;

  st_thread_t *sleep_q;      /* sleep queue for this vp */
  int sleepq_size;	      /* number of threads on sleep queue */
} _st_vp_t;


/*****************************************
 * Current vp, thread, and event system
 */

extern _st_vp_t	    _st_this_vp;
extern st_thread_t *_st_this_thread;
extern _st_eventsys_t *_st_eventsys;

#define _ST_CURRENT_THREAD()            (_st_this_thread)
#define _ST_SET_CURRENT_THREAD(_thread) (_st_this_thread = (_thread))

#define _ST_LAST_CLOCK                  (_st_this_vp.last_clock)

#define _ST_RUNQ                        (_st_this_vp.run_q)
#define _ST_IOQ                         (_st_this_vp.io_q)
#define _ST_ZOMBIEQ                     (_st_this_vp.zombie_q)
#ifdef DEBUG
#define _ST_THREADQ                     (_st_this_vp.thread_q)
#endif

#define _ST_PAGE_SIZE                   (_st_this_vp.pagesize)

#define _ST_SLEEPQ                      (_st_this_vp.sleep_q)
#define _ST_SLEEPQ_SIZE                 (_st_this_vp.sleepq_size)

#define _ST_VP_IDLE()                   (*_st_eventsys->dispatch)()


/*****************************************
 * vp queues operations
 */

#define _ST_ADD_IOQ(_pq)    ST_APPEND_LINK(&_pq.links, &_ST_IOQ)
#define _ST_DEL_IOQ(_pq)    ST_REMOVE_LINK(&_pq.links)

#define _ST_ADD_RUNQ(_thr)  ST_APPEND_LINK(&(_thr)->links, &_ST_RUNQ)
#define _ST_DEL_RUNQ(_thr)  ST_REMOVE_LINK(&(_thr)->links)

#define _ST_ADD_SLEEPQ(_thr, _timeout)  _st_add_sleep_q(_thr, _timeout)
#define _ST_DEL_SLEEPQ(_thr)		_st_del_sleep_q(_thr)

#define _ST_ADD_ZOMBIEQ(_thr)  ST_APPEND_LINK(&(_thr)->links, &_ST_ZOMBIEQ)
#define _ST_DEL_ZOMBIEQ(_thr)  ST_REMOVE_LINK(&(_thr)->links)

#ifdef DEBUG
#define _ST_ADD_THREADQ(_thr)  ST_APPEND_LINK(&(_thr)->tlink, &_ST_THREADQ)
#define _ST_DEL_THREADQ(_thr)  ST_REMOVE_LINK(&(_thr)->tlink)
#endif


/*****************************************
 * Thread states and flags
 */

#define _ST_ST_RUNNING      0 
#define _ST_ST_RUNNABLE     1
#define _ST_ST_IO_WAIT      2
#define _ST_ST_LOCK_WAIT    3
#define _ST_ST_COND_WAIT    4
#define _ST_ST_SLEEPING     5
#define _ST_ST_ZOMBIE       6
#define _ST_ST_SUSPENDED    7

#define _ST_FL_PRIMORDIAL   0x01
#define _ST_FL_IDLE_THREAD  0x02
#define _ST_FL_ON_SLEEPQ    0x04
#define _ST_FL_INTERRUPT    0x08
#define _ST_FL_TIMEDOUT     0x10


/*****************************************
 * Pointer conversion
 */

#ifndef offsetof
#define offsetof(type, identifier) ((size_t)&(((type *)0)->identifier))
#endif

/*****************************************
 * Constants
 */

#define ST_UTIME_NO_TIMEOUT ((st_utime_t) -1LL)
#define ST_DEFAULT_STACK_SIZE (128*1024)  /* Includes register stack size */

/*****************************************
 * Threads context switching
 */

static inline void MD_INIT_CONTEXT(st_thread_t* _thread, void* _sp, void (*_main)(void)) 
{											
	if (MD_SETJMP((_thread)->context))       
		_main();                                
	MD_GET_SP(_thread) = (long) (_sp);		
};

/*
 * Switch away from the current thread context by saving its state and
 * calling the thread scheduler
 */
void static inline _ST_SWITCH_CONTEXT(st_thread_t* _thread)       
{ 
    if (!MD_SETJMP((_thread)->context)) 
	{ 
      _st_vp_schedule();                  
    }                                     
}
/*
 * Restore a thread context that was saved by _ST_SWITCH_CONTEXT or
 * initialized by _ST_INIT_CONTEXT
 */
static void inline _ST_RESTORE_CONTEXT(st_thread_t* _thread)   
{
    _ST_SET_CURRENT_THREAD(_thread);   
    MD_LONGJMP((_thread)->context, 1); 
}
/*
 * Initialize the thread context preparing it to execute _main
 */
#define _ST_INIT_CONTEXT MD_INIT_CONTEXT

/*
 * Number of bytes reserved under the stack "bottom"
 */
#define _ST_STACK_PAD_SIZE MD_STACK_PAD_SIZE

/*****************************************
 * Pointer conversion
 */

#ifndef offsetof
#define offsetof(type, identifier) ((size_t)&(((type *)0)->identifier))
#endif

#define _ST_THREAD_PTR(_qp)         \
    ((st_thread_t *)((char *)(_qp) - offsetof(st_thread_t, links)))

#define _ST_THREAD_WAITQ_PTR(_qp)   \
    ((st_thread_t *)((char *)(_qp) - offsetof(st_thread_t, wait_links)))

#define _ST_THREAD_STACK_PTR(_qp)   \
    ((st_stack_t *)((char*)(_qp) - offsetof(st_stack_t, links)))

#define _ST_POLLQUEUE_PTR(_qp)      \
    ((st_pollq_t *)((char *)(_qp) - offsetof(st_pollq_t, links)))

#endif /* !__ST_COMMON_H__ */


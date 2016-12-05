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

void st_vp_schedule(void);
void st_thread_main(void);

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
} st_clist_t;

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
  st_clist_t links;
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
	st_clist_t links;          /* For putting on run/sleep/zombie queue */
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
} st_eventsys_t;


typedef struct _st_vp {
  st_thread_t *idle_thread;  /* Idle thread for this vp */
  //st_utime_t last_clock;      /* The last time we went into vp_check_clock() */

  st_clist_t run_q;          /* run queue for this vp */
  st_clist_t io_q;           /* io queue for this vp */
  st_clist_t zombie_q;       /* zombie queue for this vp */
  int pagesize;

  st_thread_t *sleep_q;      /* sleep queue for this vp */
  int sleepq_size;	      /* number of threads on sleep queue */
} st_vp_t;


/*****************************************
 * Current vp, thread, and event system
 */

extern st_vp_t			st_this_vp;
extern st_thread_t*		st_this_thread;
extern st_eventsys_t*	st_eventsys;

#define ST_CURRENT_THREAD()            (st_this_thread)
#define ST_SET_CURRENT_THREAD(_thread) (st_this_thread = (_thread))

#define ST_RUNQ                        (st_this_vp.run_q)
#define ST_IOQ                         (st_this_vp.io_q)

#define ST_VP_IDLE()                   (*st_eventsys->dispatch)()

/*****************************************
 * vp queues operations
 */
#define ST_ADD_RUNQ(_thr)  ST_APPEND_LINK(&(_thr)->links, &ST_RUNQ)
#define ST_DEL_RUNQ(_thr)  ST_REMOVE_LINK(&(_thr)->links)

/*****************************************
 * Thread states and flags
 */

#define ST_ST_RUNNING      0 
#define ST_ST_RUNNABLE     1
#define ST_ST_IO_WAIT      2
#define ST_ST_LOCK_WAIT    3
#define ST_ST_COND_WAIT    4
#define ST_ST_SLEEPING     5
#define ST_ST_ZOMBIE       6
#define ST_ST_SUSPENDED    7
#define ST_FL_PRIMORDIAL   0x01
#define ST_FL_IDLE_THREAD  0x02
#define ST_FL_ON_SLEEPQ    0x04
#define ST_FL_INTERRUPT    0x08
#define ST_FL_TIMEDOUT     0x10

#define ST_STACK_PAD_SIZE		MD_STACK_PAD_SIZE
#define ST_PAGE_SIZE			(st_this_vp.pagesize)
#define ST_UTIME_NO_TIMEOUT 	((st_utime_t) -1LL)
#define ST_DEFAULT_STACK_SIZE 	(128*1024)  /* Includes register stack size */

/*****************************************
 * Threads context switching
 */

static inline void ST_INIT_CONTEXT(st_thread_t* _thread, void* _sp) 
{											
	int ret = MD_SETJMP((_thread)->context);
	if (ret != 0)       
	{
		printf("[INIT]ret=%d, thread=%08x jmp in main process...\n", ret, (unsigned int)_thread);
		st_thread_main();                                
	}
	MD_GET_SP(_thread) = (long) (_sp);		
	printf("[INIT]ret=%d, thread=%08x jmp out...\n", ret, (unsigned int)_thread);
};

/*
 * Switch away from the current thread context by saving its state and
 * calling the thread scheduler
 */
void static inline ST_SWITCH_CONTEXT(st_thread_t* _thread)       
{
	_thread->state = ST_ST_RUNNABLE;
	int ret = MD_SETJMP((_thread)->context);
    if (ret == 0) 
	{ 
		printf("[SWITCH]ret=%d, thread=%08x go to schedule process...\n", ret, (unsigned int)_thread);
      	st_vp_schedule();                  
    } 
	printf("[SWITCH]ret=%d, thread=%08x go out...\n", ret, (unsigned int)_thread);
}
/*
 * Restore a thread context that was saved by _ST_SWITCH_CONTEXT or
 * initialized by _ST_INIT_CONTEXT
 */
static void inline ST_RESTORE_CONTEXT(st_thread_t* _thread)   
{
    ST_SET_CURRENT_THREAD(_thread);   
	printf("[RESTORE]restore to thread=%08x process...\n", (unsigned)_thread);
    MD_LONGJMP((_thread)->context, 1); 
}

/*****************************************
 * Pointer conversion
 */

#ifndef offsetof
#define offsetof(type, identifier) ((size_t)&(((type *)0)->identifier))
#endif

#define ST_THREAD_PTR(_qp)         \
    ((st_thread_t *)((char *)(_qp) - offsetof(st_thread_t, links)))

#define ST_THREAD_WAITQ_PTR(_qp)   \
    ((st_thread_t *)((char *)(_qp) - offsetof(st_thread_t, wait_links)))

#define ST_THREAD_STACK_PTR(_qp)   \
    ((st_stack_t *)((char*)(_qp) - offsetof(st_stack_t, links)))

#define ST_POLLQUEUE_PTR(_qp)      \
    ((st_pollq_t *)((char *)(_qp) - offsetof(st_pollq_t, links)))

#endif /* !__ST_COMMON_H__ */


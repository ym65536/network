/*
 * This file is derived directly from Netscape Communications Corporation,
 * and consists of extensive modifications made during the year(s) 1999-2000.
 */

#ifndef __ST_MD_H__
#define __ST_MD_H__

#if defined(ETIMEDOUT) && !defined(ETIME)
#define ETIME ETIMEDOUT
#endif

#if defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#define MAP_ANON MAP_ANONYMOUS
#endif

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

/*
 * These are properties of the linux kernel and are the same on every
 * flavor and architecture.
 */
#define MD_USE_BSD_ANON_MMAP
#define MD_ACCEPT_NB_NOT_INHERITED
#define MD_ALWAYS_UNSERIALIZED_ACCEPT
/*
 * Modern GNU/Linux is Posix.1g compliant.
 */
#define MD_HAVE_SOCKLEN_T

/*
 * All architectures and flavors of linux have the gettimeofday
 * function but if you know of a faster way, use it.
 */
#define MD_GET_UTIME()            \
  struct timeval tv;              \
  (void) gettimeofday(&tv, NULL); \
  return (tv.tv_sec * 1000000LL + tv.tv_usec)


#ifndef JB_RSP
#define JB_RSP 6
#endif

#define MD_GET_SP(_t) (_t)->context[0].__jmpbuf[JB_RSP]

#define MD_INIT_CONTEXT(_thread, _sp, _main)  
/*static inline void MD_INIT_CONTEXT(_thread, _sp, _main) 
{											
	if (MD_SETJMP((_thread)->context))       
		_main();                                
	MD_GET_SP(_thread) = (long) (_sp);		
};
*/
#define MT_SETJMP(env) _st_md_cxt_save(env)
#define MT_LONGJMP(env, val) _st_md_cxt_restore(env, val)

extern int _st_md_cxt_save(jmp_buf env);
extern void _st_md_cxt_restore(jmp_buf env, int val);

#ifndef MD_STACK_PAD_SIZE
#define MD_STACK_PAD_SIZE 128
#endif

#endif /* !__ST_MD_H__ */


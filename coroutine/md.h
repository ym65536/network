/*
 * This file is derived directly from Netscape Communications Corporation,
 * and consists of extensive modifications made during the year(s) 1999-2000.
 */

#ifndef __ST_MD_H__
#define __ST_MD_H__

#define MD_GET_UTIME()            \
  struct timeval tv;              \
  (void) gettimeofday(&tv, NULL); \
  return (tv.tv_sec * 1000000LL + tv.tv_usec)


#define JB_SP  4

#define MD_GET_SP(_t) (_t)->context[0].__jmpbuf[JB_SP]

#define MD_SETJMP(env) st_md_cxt_save(env)
#define MD_LONGJMP(env, val) st_md_cxt_restore(env, val)

extern int st_md_cxt_save(jmp_buf env);
extern void st_md_cxt_restore(jmp_buf env, int val);

#ifndef MD_STACK_PAD_SIZE
#define MD_STACK_PAD_SIZE 128
#endif

#endif /* !__ST_MD_H__ */


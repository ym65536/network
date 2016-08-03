#include "common.h"

#define MAX_THREAD (10 * 1024)
//#define MAX_THREAD (4)

struct thread_data
{
	int seqno;
	char data[0x100];
	st_thread_t* stid;
};
struct thread_data tdata[MAX_THREAD];

void thread(void* args)
{
	struct thread_data* pt = (struct thread_data* )args;
	st_thread_t* ctid = ST_CURRENT_THREAD();
	printf("ENTER: data=%s, seqno=%d, ctid=%08x, stid1=%08x\n", pt->data, pt->seqno, (unsigned int)ctid, (unsigned int)pt->stid);

	ST_SWITCH_CONTEXT(ctid);
	printf("OUTER: data=%s, seqno=%d, ctid=%08x, stid1=%08x\n", pt->data, pt->seqno, (unsigned int)ctid, (unsigned int)pt->stid);
}

int main(void)
{
	if (st_init() < 0)
	{
		printf("state thread init fail!\n");
		return -1;
	}
	 
	int i = 0;
	for (i = 0; i < MAX_THREAD; i++)
	{
		tdata[i].seqno = i;
		sprintf(tdata[i].data, "I am in thread %d\n", i);
		tdata[i].stid = (st_thread_t* )st_thread_create(thread, (void* )&tdata[i]);
	}

	ST_RESTORE_CONTEXT(tdata[0].stid);

	return 0;
}

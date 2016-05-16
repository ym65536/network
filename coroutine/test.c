#include "common.h"

st_thread_t* stid1 = NULL;
st_thread_t* stid2 = NULL;

void thread1(void* args)
{
	char* data = (char* )args;
	st_thread_t* ctid = ST_CURRENT_THREAD();
	static int seq = 0;
	printf("T1 ENTER: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
	seq++;

	ST_SWITCH_CONTEXT(ctid);
	printf("T1 OUT: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
}

void thread2(void* args)
{
	char* data = (char* )args;
	st_thread_t* ctid = ST_CURRENT_THREAD();
	static int seq = 0;
	printf("T2 ENTER: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
	seq++;

	ST_SWITCH_CONTEXT(ctid);
	printf("T2 OUT: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
}

int main(void)
{
	if (st_init() < 0)
	{
		printf("state thread init fail!\n");
		return -1;
	}

	stid1 = (st_thread_t* )st_thread_create(thread1, (void* )"this is thread1!");
	stid2 = (st_thread_t* )st_thread_create(thread2, (void* )"this is thread2!");

	ST_RESTORE_CONTEXT(stid1);

	return 0;
}

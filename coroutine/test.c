#include "common.h"

st_thread_t* stid1 = NULL;
st_thread_t* stid2 = NULL;

void thread1(void* args)
{
	char* data = (char* )args;
	st_thread_t* ctid = _ST_CURRENT_THREAD();
	static int seq = 0;
	printf("T1: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
	seq++;

	_ST_SWITCH_CONTEXT(ctid);
}

void thread2(void* args)
{
	char* data = (char* )args;
	st_thread_t* ctid = _ST_CURRENT_THREAD();
	static int seq = 0;
	printf("T2: data=%s, seqno=%d, ctid=%08x, stid1=%08x, stid2=%08x\n", data, seq, (unsigned int)ctid, (unsigned int)stid1, (unsigned int)stid2);
	seq++;

	_ST_SWITCH_CONTEXT(ctid);
}

int main(void)
{
	if (st_init() < 0)
	{
		printf("state thread init fail!\n");
		return -1;
	}

	stid1 = st_thread_create(thread1, (void* )"this is thread1!");
	stid2 = st_thread_create(thread2, (void* )"this is thread2!");

	_ST_RESTORE_CONTEXT(stid1);

	return 0;
}

#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;
int times = 0;

void second(int* k)
{
    printf("second times = %d\n", ++(*k));
    longjmp(buf, 65536);
}

void first(int* k)
{
    printf("first times = %d\n", ++(*k));
    second(k);
}

int main(void)
{
    int ret = setjmp(buf);
    if (ret == 0)
    {
        printf("1. ret is %d\n", ret);
        first(&times);
    }
    else
    {
        printf("2. ret is %d\n", ret);
    }
}

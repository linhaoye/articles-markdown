#include "common.h"
#include "memory.h"

int main(void)
{
	char *f1 = "/root/src/b.txt";
	smem_t smt;

	share_memory_new(&smt, 128, f1);
	memcpy(smt.addr, "hello world!", 13);
	share_memory_delete(&smt);

	return 0;
}

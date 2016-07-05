
#include <assert.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "common.h"
#include "memory.h"

void *share_memory_new(smem_t *smt, size_t size, char *file)
{
	void *m;
	int fd = 0;
	memset(smt, 0, sizeof(*smt));

	if (file == NULL) {
		file = "/dev/zero";
	}

	if ((fd = open(file, O_RDWR)) == -1) {
		err_exit("open()");
	}

	if ((m = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0)) == (caddr_t)-1) {
		err_exit("mmap()");
	}

	strncpy(smt->file, file, MAP_FILE_SIZE);
	smt->size = size;
	smt->fd = fd;
	smt->addr = m;

	return m;
}

int share_memory_delete(smem_t *smt)
{
	assert(smt->addr != NULL);

	close(smt->fd);
	return munmap(smt->addr, smt->size);
}

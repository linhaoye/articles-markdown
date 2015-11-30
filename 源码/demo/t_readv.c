/*************************************************************************
    > File Name: t_readv.c
    > Author: MrLin
    > Mail: 714480119@qq.com 
    > Created Time: Tue 18 Aug 2015 05:51:56 PM CST
 ************************************************************************/

#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define STR_SIZE	100

struct vec {
	char buf[128];
	size_t len;	
};

int main(int argc, char *argv[])
{
	int fd;
	struct iovec iov[3];
	struct vec myVec;
	int x;
	char str[STR_SIZE];
	ssize_t numRead, totRequired;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		usageErr("%s file\n", argv[0]);

	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		errExit("open");

	totRequired = 0;
	iov[0].iov_base = &myVec;
	iov[0].iov_len = sizeof(struct vec);


	totRequired += iov[0].iov_len;
	
	iov[1].iov_base = &x;
	iov[1].iov_len = sizeof(x);
	totRequired += iov[1].iov_len;
	
	iov[2].iov_base = str;
	iov[2].iov_len = STR_SIZE;
	totRequired += iov[2].iov_len;

	while ( (numRead = readv(fd, iov, 3)) != 0)
	{
		memcpy(&myVec, iov[0].iov_base, iov[0].iov_len);
		memcpy(&x, iov[1].iov_base, iov[1].iov_len);
		memcpy(str, iov[2].iov_base, iov[2].iov_len);

		printf ("num read: %d %d %s %s\n",numRead, x, myVec.buf, str);
	}
	if (numRead == -1)
		errExit("readv");

	if (numRead < totRequired)
		printf ("Read fewer bytes than requested\n");
	printf ("total bytes requested: %ld; bytes read: %ld\n", (long)totRequired, (long)numRead);

	close(fd);

	exit(EXIT_SUCCESS);
}

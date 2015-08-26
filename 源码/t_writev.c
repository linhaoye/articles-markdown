/*************************************************************************
    > File Name: t_writev.c
    > Author: MrLin
    > Mail: 714480119@qq.com 
    > Created Time: Fri 21 Aug 2015 10:23:20 AM CST
 ************************************************************************/

#include <sys/uio.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

struct vec {
	char buf[128];
	size_t len;
};

#define	STR_SIZE 100
#define WRT_SIZE 100

int
main(int argc, char *argv[])
{
	int fd;
	struct iovec iov[3];
	struct vec myVec;
	int x;
	char str[STR_SIZE];
	ssize_t numWrite, totRequired;

	if (argc != 2 || strcmp(argv[1], "--help") == 0)
		usageErr("%s file\n", argv[0]);

	fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0664);
	if (fd == -1)
		errExit("open");

	int i;
	totRequired = 0;
	numWrite = 0;

	for (i = 0; i < WRT_SIZE; i++)
	{

		sprintf(myVec.buf, "hello world %d!", i);
		myVec.len = strlen(myVec.buf);
		x = i;
		sprintf(str, "another line %d", i);


		iov[0].iov_base = &myVec;
		iov[0].iov_len = sizeof(struct vec);
		totRequired += iov[0].iov_len;

		iov[1].iov_base = &x;
		iov[1].iov_len = sizeof(x);
		totRequired += iov[1].iov_len;

		iov[2].iov_base = str;
		iov[2].iov_len = STR_SIZE;
		totRequired += iov[2].iov_len;

		numWrite += writev(fd, iov, 3);
	}


	if (numWrite == -1)
		errExit("writev");

	if (numWrite < totRequired)
		printf ("Write fewer bytes than request\n");

	printf ("total bytes request: %ld; bytes write: %ld", (long)totRequired, (long)numWrite);

	close(fd);

	exit(EXIT_SUCCESS);
}

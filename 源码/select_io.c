/*************************************************************************
    > File Name: select_io.c
    > Author: MrLin
    > Mail: 714480119@qq.com 
    > Created Time: Fri 14 Aug 2015 04:07:58 PM CST
 ************************************************************************/

#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include "tlpi_hdr.h"

int main(int argc, char *argv[])
{
	struct timeval timeout;
	char buf[10];
	fd_set readfds;
	int nread, nfds, ready, fd;

	while(1) {
		timeout.tv_sec = 20L;
		timeout.tv_usec = 0;

		fd = 0;
		nfds = fd + 1;
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		ready = select (nfds, &readfds, NULL, NULL, &timeout);

		if (ready == -1 && errno == EINTR) {
			continue;
		} else if  (ready == -1) {
			errMsg("select error\n");
		}
		for (fd = 0; fd < nfds; fd++)
		{
			if (FD_ISSET(fd, &readfds)) {
				nread = read(fd, buf, 9);
				buf[nread] = '\0';
				puts(buf);
			}
		}
	}

	exit(EXIT_SUCCESS);
}

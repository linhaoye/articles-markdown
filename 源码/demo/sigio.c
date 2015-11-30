/*************************************************************************
    > File Name: sigio.c
    > Author: MrLin
    > Mail: 714480119@qq.com 
    > Created Time: Mon 17 Aug 2015 05:16:47 PM CST
 ************************************************************************/

#include <signal.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

static int g_fd;

static void sigio_handler(int signum)
{
	char buf[8] = {0};

	if (read(g_fd, buf, 7) < 0)
		errMsg("read errorn\n");
	else
		printf("sigio recv: %s\n", buf);
}

int main(int argc, char *argv[])
{
	struct sigaction act;
	int flags, i=1, fds[2];
	pid_t pid;

	if (pipe(fds) < 0) {
		errExit("pipe errors\n");
	}

	if ( (pid = fork()) > 0) {
		close(fds[1]);
		dup2(fds[0], g_fd);

		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_RESTART;
		act.sa_handler = sigio_handler;

		if (sigaction(SIGIO, &act, NULL)) {
			errExit("sigaction error\n");
		}

		if (fcntl(g_fd, F_SETOWN, getpid()) == -1) {
			errExit("fcntl F_SETOWN error\n");
		}

		flags = fcntl(g_fd, F_GETFL);
		if (fcntl(g_fd, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
			errExit("fcntl F_GETFL error\n");
		}
		while (1)
			sleep(10);
	} else {
		char buf[20] = {0};
		close(fds[0]);

		for (i =0; i < 3; i++) {
			snprintf(buf, 20, "this is loop %d", i);
			write(fds[1], buf, strlen(buf));
			printf("loop %d\n", i);
			sleep(3);
		}
	}

	return 0;
}

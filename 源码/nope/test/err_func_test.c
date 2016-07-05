#include <sys/wait.h>
#include "common.h"

int main(void)
{
	int status;
	pid_t pid;

	TRACE("YEAR!!!");

	if ((pid = fork()) == -1) {
		err_exit("fork");
	} else if (pid == 0) {
		err_msg("child pid:%d!", getpid());

		_err_exit("child exit!");
	} else { /* parent */
		err_msg("parent goes here!");
	}

	if (wait(&status) < 0) {
		err_exit("wait");
	}

	err_msg("child exit status:%d", status);
	err_exit_en(12,"hello %s\n", "world!");

	return 0;
}
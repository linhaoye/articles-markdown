#include <sys/wait.h>
#include "common.h"
#include "pipe.h"

static char data[] = "$#@$$$______||||||||szdf";

int main(void)
{
	int status;
	pid_t pid;
	char buf[128] = {0};

	pipe_t pip;
	pipe_open(&pip, 1);

	if ( (pid = fork()) == -1) {
		err_exit("fork");
	} else if (pid == 0) {
		//close write end
		close(pipe_fd(&pip, 1));

		pipe_read(&pip, buf, sizeof(buf));
		printf("read from parent: %s\n", buf);

		exit(0);
	} else { /* parent */
		//close read end
		close(pipe_fd(&pip, 0));

		pipe_write(&pip, data, sizeof(data));
		printf("parent write ok!\n");
	}

	if (wait(&status) == -1) {
		err_exit("wait");
	}

	printf("child exit status: %d\n", status);

	return 0;
}

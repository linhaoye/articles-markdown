#include <fcntl.h>
#include <unistd.h>

void setfl(int fd, int fl)
{
	int flags = 0;

	if ((flags = fcntl(fd, F_GETFL)) == -1) {
		err_exit("fcntl");
	}

	flags |= fl;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		err_exit("fcntl");
	}
}

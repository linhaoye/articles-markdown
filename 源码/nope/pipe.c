#include <fcntl.h>
#include "common.h"
#include "pipe.h"

typedef struct _pipe {
	int fd[2];
} _pipe;

void pipe_open(pipe_t *pip, int blocking)
{
	_pipe *_pipe = malloc(sizeof (*_pipe));

	if (_pipe == NULL) {
		err_exit("malloc()");
	}

	memset(_pipe, 0, sizeof (*_pipe));

	if (pipe(_pipe->fd) < 0) {
		err_exit("pipe()");
	}

	if (blocking == 0) {
		setfl(_pipe->fd[0], O_NONBLOCK);
		setfl(_pipe->fd[1], O_NONBLOCK);
	}

	pip->object = _pipe;
	pip->blocking = blocking;
}

int pipe_read(pipe_t *pip, void *data, size_t sz)
{
	_pipe *p = pip->object;

	return read(p->fd[0], data, sz);
}

int pipe_write(pipe_t *pip, void *data, size_t sz)
{
	_pipe *p = pip->object;

	return write(p->fd[1], data, sz);
}

int pipe_fd(pipe_t *pip, int type)
{
	_pipe *p = pip->object;

	return type == 0 ? p->fd[0] : p->fd[1];
}

void pipe_close(pipe_t *pip)
{
	_pipe *p = pip->object;

	close(p->fd[0]);
	close(p->fd[1]);
	free(p);
}

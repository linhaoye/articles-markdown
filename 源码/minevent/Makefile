CFLAGS = -Wall -g -O0

all:
	gcc ${CFLAGS} -I ./include -c src/event.c
	gcc ${CFLAGS} -I ./include -c src/epoll.c
	ar rcs ./lib/libminevent.a event.o epoll.o

clean:
	rm -f *.o *.fifo ./lib/*.a
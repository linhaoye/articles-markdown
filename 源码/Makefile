a.out: sigio.o  error_functions.o get_num.o
	gcc -o a.out sigio.o error_functions.o get_num.o

sigio.o: sigio.c tlpi_hdr.h
	gcc -c sigio.c
a.out: sigio.o  error_functions.o get_num.o
	gcc -o a.out sigio.o error_functions.o get_num.o
error_functions.o: error_functions.c ename.c.inc error_functions.h tlpi_hdr.h
	gcc -c error_functions.c
get_num.o: get_num.c get_num.h
	gcc -c get_num.c

clean:
	rm  sigio.o error_functions.o get_num.o

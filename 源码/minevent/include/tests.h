#ifndef __TESTS_H__
#define __TESTS_H__

#define unit_test(x) 			int unit_test_##x(unit_test *ut)
#define unit_test_setup(x,n,t)	_unit_test_setup(unit_test_##x, #x, n, t)

typedef struct _unit_test
{
	int argc;
	char **argv;
}unit_test;

typedef int (*unit_test_func)(unit_test *ut);

void _unit_test_setup(unit_test_func func, char *func_name, int run_times, char *comment);
int unit_test_run(unit_test *ut);

#endif

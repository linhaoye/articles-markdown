#ifndef __ERR_FUNC_H__
#define __ERR_FUNC_H__

void err_msg(const char *fmt, ...);

/* This macro stops 'gcc -Wall' complaining that "control reaches
   end of non-void function" if we use the following functions to
   terminate main() or some other non-void function. */

#ifdef __GNUC__
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

void _err_exit(const char *fmt, ...) NORETURN;

void err_exit(const char *fmt, ...) NORETURN;

void err_exit_en(int errn, const char *fmt, ...) NORETURN;

void fatal(const char *fmt, ...) NORETURN;

#ifdef TRACE
#undef TRACE
#endif

#include <time.h>
#define __QUOTE(x) # x
#define _QUOTE(x) __QUOTE(x)

#define TRACE(fmt, ...) do { \
	time_t t = time(NULL); \
	struct tm *dm = localtime(&t); \
	err_msg("[%02d:%02d:%02d]%s:[" _QUOTE(__LINE__) "]\t 	%-26s:" \
		fmt, \
		dm->tm_hour, \
		dm->tm_min, \
		dm->tm_sec, \
		__FILE__, \
		__func__, \
		## __VA_ARGS__); \
} while(0)

#endif

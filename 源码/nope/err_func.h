#ifndef __ERR_FUNC_H__
#define __ERR_FUNC_H__

void err_msg(const char *fmt, ...);

#ifdef __GNUC__
#define NORETURN __attribute__ ((__noreturn))
#else
#define NORETURN
#endif

void _err_exit(const char *fmt, ...) NORETURN;

void err_exit(const char *fmt, ...) NORETURN;

void err_exit_en(int errn, const char *fmt, ...) NORETURN;

void fatal(const char *fmt, ...) NORETURN;

#endif;
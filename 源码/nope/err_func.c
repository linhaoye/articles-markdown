/* err_func.c

   Some standard error handling routines used by various programs.
*/


#include <stdarg.h>
#include "common.h"
#include "err_func.h"
#include "ename.c.inc"

#ifdef __GNUC__
__attribute__ ((__noreturn__))
#endif

static void terminate(Boolean useExit3)
{
	char *s;

    /* Dump core if EF_DUMPCORE environment variable is defined and
       is a nonempty string; otherwise call exit(3) or _exit(2),
       depending on the value of 'useExit3'. */

	s = getenv("EF_DUMPCORE");

	if (s != NULL && *s != '\0') {
		abort();
	} else if (useExit3) {
		exit(EXIT_FAILURE);
	} else {
		_exit(EXIT_FAILURE);
	}
}

/* Diagnose 'errno' error by:

      * outputting a string containing the error name (if available
        in 'ename' array) corresponding to the value in 'err', along
        with the corresponding error message from strerror(), and

      * outputting the caller-supplied error message specified in
        'format' and 'ap'. */

static void output_error(Boolean useErr, int err, Boolean flushStdout,
	const char *fmt, va_list ap)
{
#define BUF_SIZE 500
	char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];

	vsnprintf(userMsg, BUF_SIZE, fmt, ap);

	if (useErr) {
		snprintf(errText, BUF_SIZE, " [%s %s]",
				(err > 0 && err <= MAX_ENAME) ?
				ename[err] : "?UNKNOWN?", strerror(err));
	} else {
		snprintf(errText, BUF_SIZE, ":");
	}

	snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);

	if (flushStdout) {
		fflush(stdout);
	}
	fputs(buf, stderr);
	fflush(stderr);
}

/* Display error message including 'errno' diagnostic, and
   return to caller */

void err_msg(const char *fmt, ...)
{
	va_list argList;
	int savedErrno;

	savedErrno = errno;

	va_start(argList, fmt);
	output_error(TRUE, errno, TRUE, fmt, argList);
	va_end(argList);

	errno = savedErrno;
}

/* Display error message including 'errno' diagnostic, and
   terminate the process */

void err_exit(const char *fmt, ...)
{
	va_list argList;

	va_start(argList, fmt);
	output_error(TRUE, errno, TRUE, fmt, argList);
	va_end(argList);

	terminate(TRUE);
}

/* Display error message including 'errno' diagnostic, and
   terminate the process by calling _exit().

   The relationship between this function and errExit() is analogous
   to that between _exit(2) and exit(3): unlike errExit(), this
   function does not flush stdout and calls _exit(2) to terminate the
   process (rather than exit(3), which would cause exit handlers to be
   invoked).

   These differences make this function especially useful in a library
   function that creates a child process that must then terminate
   because of an error: the child must terminate without flushing
   stdio buffers that were partially filled by the caller and without
   invoking exit handlers that were established by the caller. */

void _err_exit(const char *fmt, ...)
{
	va_list argList;

	va_start(argList, fmt);
	output_error(TRUE, errno, FALSE, fmt, argList);
	va_end(argList);

	terminate(FALSE);
}

/* The following function does the same as errExit(), but expects
   the error number in 'errnum' */

void err_exit_en(int errn, const char *fmt, ...)
{
	va_list argList;

	va_start(argList, fmt);
	output_error(TRUE, errn, TRUE, fmt, argList);
	va_end(argList);

	terminate(TRUE);
}

/* Print an error message (without an 'errno' diagnostic) */

void fatal(const char *fmt, ...)
{
	va_list argList;

	va_start(argList, fmt);	
	output_error(FALSE, 0, TRUE, fmt, argList);
	va_end(argList);

	terminate(TRUE);
}

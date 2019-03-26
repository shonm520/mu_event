
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#define MAXLINE	128

#define DBG_PRINTF debug_print

static void	debug_print(int, const char *, va_list);

/* 系统调用非致命错误
 * 打印用户信息 + errno信息后返回
 */
void debug_ret(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	DBG_PRINTF(1, fmt, ap);
	va_end(ap);
}

/* 系统调用致命错误
 * 打印用户信息 + errno信息后终止进程
 */
void debug_sys(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	DBG_PRINTF(1, fmt, ap);
	va_end(ap);
	exit(1);
}

/* 非系统调用致命错误
 * 打印用户信息后终止进程
 */
void debug_quit(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	DBG_PRINTF(0, fmt, ap);
	va_end(ap);
	exit(1);
}

/* 普通用户信息 */
void debug_msg(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	DBG_PRINTF(0, fmt, ap);
	va_end(ap);
}

/* 打印核心函数 */
static void debug_print(int errnoflag, const char *fmt, va_list ap)
{
	int	errno_save, n;
	char buf[MAXLINE + 1];

	errno_save = errno;

	vsnprintf(buf, MAXLINE, fmt, ap);	/* safe */

	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
	strcat(buf, "\n");

	fflush(stderr);
	fputs(buf, stderr);	/* no newline */
	fflush(stderr);
}


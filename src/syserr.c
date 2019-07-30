/*************************************************************************
** syserr.c for USBManager 
** Copyright (c) 2012, 2013 by YAO Zhen-Guo <yaozhenguo2006@gmail.com>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** (See the included file COPYING)
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

static void err_doit(int, int, const char *, va_list);

/*
 * Nonfatal error related to a system call
 * Print a message and return
 */
void err_ret(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error related to a system call
 * Print a message and terminate
 */
void err_sys(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}
/*
 * Fatal errno unrelated to a system call
 * Error code passed as explict parameter
 * Print a message and terminate
 */
void err_exit(int error, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, error, fmt, ap);
	va_end(ap);
	exit(1);
}

/* Fatal error related to a system call
 * Print a message, dump core, and terminate
 */
void err_dump(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	abort();
	exit(1);
}
/* Nonfatal error unrelated to a system call
 * Print a message and return
 */
void err_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}
/* Fatal error unrelated to a system call
 * Print a message and terminate
 */
void err_quit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
	exit(1);
}
/*
 * Print a message and return to caller
 * Caller specifies "errnoflag"
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[4096];

	vsnprintf(buf, 4096, fmt, ap);
	if (errnoflag)
		snprintf(buf + strlen(buf), 4096 - strlen(buf), ": %s",
		 strerror(error));
	strcat(buf, "\n");
	fflush(stdout); 
	fputs(buf, stderr);
	fflush(NULL);
}


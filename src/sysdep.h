#ifndef MUD_SYSDEP_H
#define MUD_SYSDEP_H
/* ************************************************************************
*   File: sysdep.h                                      Part of CircleMUD *
*  Usage: machine-specific defs based on values in conf.h (from configure)*
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

//#define TEST_CODE

#ifdef _NIX_
#define CIRCLE_UNIX
#define POSIX
#define CIRCLE_CRYPT //Убрать если не нужно криптование паролей.
#else
#define CIRCLE_WINDOWS
#endif

#define HAVE_ZLIB

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _NIX_
#include <arpa/telnet.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#else
#include <process.h>
#include "telnet.h"
#define socklen_t int
#endif


#include <ctype.h>
#include <stdarg.h>
#include <vector>

#include <strstream>
#include <iostream>
#include <fstream>

#include <math.h>

#include <string.h>
#include <strings.h>

#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef CIRCLE_CRYPT
#ifdef __FreeBSD__
#include <unistd.h>
#else
#include <crypt.h>
#endif
#endif /* CIRCLE_CRYPT */

#include <limits.h>
#include <errno.h>
#include <assert.h>

#ifdef _MINGW_
#include "./zlib/zlib.h"
#else
#include <zlib.h>
#endif

#include <signal.h>
#include <fcntl.h>

/* Basic system dependencies *******************************************/

#if !defined(__GNUC__)
#define __attribute__(x) /* nothing */
#endif

/* Socket/header miscellany. */


#ifdef _MINGW_
# ifndef _WINSOCK2API_ /* Winsock1 and Winsock 2 conflict. */
#  include <winsock.h>
# endif
#endif

# ifndef FD_SETSIZE /* MSVC 6 is reported to have 64. */
#  define FD_SETSIZE  1024
# endif

/* SOCKET -- must be after the winsock.h #include. */
#ifdef CIRCLE_WINDOWS
# define CLOSE_SOCKET(sock) closesocket(sock)
typedef SOCKET  socket_t;
#else
# define CLOSE_SOCKET(sock) close(sock)
typedef int   socket_t;
#endif

/* Guess if we have the getrlimit()/setrlimit() functions */
#if defined(RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#define HAS_RLIMIT
#if !defined (RLIMIT_NOFILE)
# define RLIMIT_NOFILE RLIMIT_OFILE
#endif
#endif


/* Make sure we have STDERR_FILENO */
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

/* Make sure we have STDOUT_FILENO too. */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif


/* Function prototypes ************************************************/

/*
 * For reasons that perplex me, the header files of many OS's do not contain
 * function prototypes for the standard C library functions.  This produces
 * annoying warning messages (sometimes, a huge number of them) on such OS's
 * when compiling with gcc's -Wall.
 *
 * Some versions of CircleMUD prior to 3.0 patchlevel 9 attempted to
 * include prototypes taken from OS man pages for a large number of
 * OS's in the header files.  I now think such an approach is a bad
 * idea: maintaining that list is very difficult and time-consuming,
 * and when new revisions of OS's are released with new header files,
 * Circle can break if the prototypes contained in Circle's .h files
 * differs from the new OS header files; for example, Circle 3.0
 * patchlevel 8 failed with compiler errors under Solaris 2.5 and
 * Linux 1.3.xx whereas under previous revisions of those OS's it had
 * been fine.
 *
 * Thus, to silence the compiler warnings but still maintain some level of
 * portability (albiet at the expense of worse error checking in the code),
 * my solution is to define a "typeless" function prototype for all problem
 * functions that have not already been prototyped by the OS. --JE
 *
 * 20 Mar 96: My quest is not yet over.  These definitions still cause
 * clashes with some compilers.  Therefore, we only use these prototypes
 * if we're using gcc (which makes sense, since they're only here for gcc's
 * -Wall option in the first place), and configure tells gcc to use
 * -fno-strict-prototypes, so that these definitions don't clash with
 * previous prototypes.
 *
 * 4 June 96: The quest continues.  OSF/1 still doesn't like these
 * prototypes, even with gcc and -fno-strict-prototypes.  I've created
 * the constant NO_LIBRARY_PROTOTYPES to allow people to turn off the
 * prototyping.
 *
 * 27 Oct 97: This is driving me crazy but I think I've finally come
 * up with the solution that will work.  I've changed the configure
 * script to detect which prototypes exist already; this header file
 * only prototypes functions that aren't already prototyped by the
 * system headers.  A clash should be impossible.  This should give us
 * our strong type-checking back.  This should be the last word on
 * this issue!
 */

#ifndef NO_LIBRARY_PROTOTYPES

#ifdef NEED_ATOI_PROTO
int atoi(const char *str);
#endif

#ifdef NEED_ATOL_PROTO
long atol(const char *str);
#endif

/*
 * bzero is deprecated - use memset() instead.  Not directly used in Circle
 * but the prototype needed for FD_xxx macros on some machines.
 */
#ifdef NEED_BZERO_PROTO
void bzero(char *b, int length);
#endif

#ifdef NEED_CRYPT_PROTO
char *crypt(const char *key, const char *salt);
#endif

#ifdef NEED_FCLOSE_PROTO
int fclose(FILE *stream);
#endif

#ifdef NEED_FDOPEN_PROTO
FILE *fdopen(int fd, const char *mode);
#endif

#ifdef NEED_FFLUSH_PROTO
int fflush(FILE *stream);
#endif

#ifdef NEED_FPRINTF_PROTO
int fprintf(FILE *strm, const char *format, /* args */ ...);
#endif

#ifdef NEED_FREAD_PROTO
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);
#endif

#ifdef NEED_FSCANF_PROTO
int fscanf(FILE *strm, const char *format, ...);
#endif

#ifdef NEED_FSEEK_PROTO
int fseek(FILE *stream, long offset, int ptrname);
#endif

#ifdef NEED_FWRITE_PROTO
size_t fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream);
#endif

#ifdef NEED_GETPID_PROTO
pid_t getpid(void);
#endif

#ifdef NEED_QSORT_PROTO
void qsort(void *base, size_t nel, size_t width,
           int (*compar)(const void *, const void *));
#endif

#ifdef NEED_REWIND_PROTO
void rewind(FILE *stream);
#endif

#ifdef NEED_SPRINTF_PROTO
int sprintf(char *s, const char *format, /* args */ ...);
#endif

#ifdef NEED_SSCANF_PROTO
int sscanf(const char *s, const char *format, ...);
#endif

#ifdef NEED_SYSTEM_PROTO
int system(const char *string);
#endif

#ifdef NEED_TIME_PROTO
time_t time(time_t *tloc);
#endif

#ifdef NEED_UNLINK_PROTO
int unlink(const char *path);
#endif

#ifdef NEED_REMOVE_PROTO
int remove(const char *path);
#endif

/* Function prototypes that are only used in comm.c and some of the utils */

#if defined(__COMM_C__) || defined(CIRCLE_UTIL)

#ifdef NEED_ACCEPT_PROTO
int accept(socket_t s, struct sockaddr *addr, int *addrlen);
#endif

#ifdef NEED_BIND_PROTO
int bind(socket_t s, const struct sockaddr *name, int namelen);
#endif

#ifdef NEED_CHDIR_PROTO
int chdir(const char *path);
#endif

#ifdef NEED_CLOSE_PROTO
int close(int fildes);
#endif

#ifdef NEED_FCNTL_PROTO
int fcntl(int fildes, int cmd, /* arg */ ...);
#endif

#ifdef NEED_FPUTC_PROTO
int fputc(char c, FILE *stream);
#endif

#ifdef NEED_FPUTS_PROTO
int fputs(const char *s, FILE *stream);
#endif

#ifdef NEED_GETPEERNAME_PROTO
int getpeername(socket_t s, struct sockaddr *name, int *namelen);
#endif

#if defined(HAS_RLIMIT) && defined(NEED_GETRLIMIT_PROTO)
int getrlimit(int resource, struct rlimit *rlp);
#endif

#ifdef NEED_GETSOCKNAME_PROTO
int getsockname(socket_t s, struct sockaddr *name, int *namelen);
#endif

#ifdef NEED_GETTIMEOFDAY_PROTO
int gettimeofday(struct timeval *tp, void *);
#endif

#ifdef NEED_HTONL_PROTO
ulong htonl(u_long hostlong);
#endif

#ifdef NEED_HTONS_PROTO
u_short htons(u_short hostshort);
#endif

#if defined(HAVE_INET_ADDR) && defined(NEED_INET_ADDR_PROTO)
unsigned long int inet_addr(const char *cp);
#endif

#if defined(HAVE_INET_ATON) && defined(NEED_INET_ATON_PROTO)
int inet_aton(const char *cp, struct in_addr *inp);
#endif

#ifdef NEED_INET_NTOA_PROTO
char *inet_ntoa(const struct in_addr in);
#endif

#ifdef NEED_LISTEN_PROTO
int listen(socket_t s, int backlog);
#endif

#ifdef NEED_NTOHL_PROTO
u_long ntohl(u_long netlong);
#endif

#ifdef NEED_PRINTF_PROTO
int printf(char *format, ...);
#endif

#ifdef NEED_READ_PROTO
ssize_t read(int fildes, void *buf, size_t nbyte);
#endif

#ifdef NEED_SELECT_PROTO
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
#endif

#ifdef NEED_SETITIMER_PROTO
int setitimer(int which, const struct itimerval *value,
              struct itimerval *ovalue);
#endif

#if defined(HAS_RLIMIT) && defined(NEED_SETRLIMIT_PROTO)
int setrlimit(int resource, const struct rlimit *rlp);
#endif

#ifdef NEED_SETSOCKOPT_PROTO
int setsockopt(socket_t s, int level, int optname, const char *optval,
               int optlen);
#endif

#ifdef NEED_SOCKET_PROTO
int socket(int domain, int type, int protocol);
#endif

#ifdef NEED_WRITE_PROTO
ssize_t write(int fildes, const void *buf, size_t nbyte);
#endif

#endif /* __COMM_C__ */

#endif /* NO_LIBRARY_PROTOTYPES */

#endif /* MUD_SYSDEP_H */

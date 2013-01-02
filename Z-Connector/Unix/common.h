#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <openssl/ssl.h>

#ifdef WIN32

#include <io.h>
#include <WinSock.h>

#else

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#if defined(__linux__) && (defined(__i386__) || defined(__x86_64__))
#define __sd_linux_use_directIO__ 1
#include <sys/io.h>
#endif
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

typedef int SOCKET;
#define SOCKET_ERROR -1
#define closesocket(x) close(x)

#define _sleep(x) usleep(x * 1000)

#ifndef O_BINARY
#define O_BINARY 0
#endif

#endif

#endif
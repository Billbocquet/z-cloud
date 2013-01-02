#ifndef _UTIL_H_
#define _UTIL_H_

// reads socket data with timeout
int ReadSockTimeout(SOCKET sock, SSL *session, char *buffer, size_t size, long timeout);

// reads port data with timeout
int ReadPortTimeout(int device, char *buffer, size_t size, long timeout);

#endif
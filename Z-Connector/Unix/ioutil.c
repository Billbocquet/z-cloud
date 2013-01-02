#include "common.h"
#include "ioutil.h"
#include <openssl/err.h>
#include "ssl.h"
#include "log.h"

int ReadSockTimeout(SOCKET sock, SSL *session, char *buffer, size_t size, long timeout)
{
	fd_set set;
	struct timeval tv = { 0, timeout };
	int result;

	FD_ZERO(&set);
	FD_SET(sock, &set);

	if (0 >= select(sock + 1, &set, NULL, NULL, &tv))
		return 0;

	if (session != NULL)
	{
		result = SSL_read(session, buffer, size);

		switch (SSL_get_error(session, result))
		{
		case SSL_ERROR_NONE:
			break;

		case SSL_ERROR_ZERO_RETURN:
			DBG("Server has closed the connection\n");
			result = 0;
			break;

		default:
			DEFAULT_HANDLER("Error reading data from server", session, result);
			result = 0;
			break;
		}
	}
	else
		result = recv(sock, buffer, size, 0);

	if (result == 0)
	{
		ERR("Socket has gone\n");
		return -1;
	}

	return result;
}

int ReadPortTimeout(int device, char *buffer, size_t size, long timeout)
{
	int result;

#ifdef WIN32
	COMSTAT stats;
	DWORD error;
	HANDLE hFile = (HANDLE)_get_osfhandle(device);

	if (!ClearCommError(hFile, NULL, &stats))
	{
		error = GetLastError();
		DBG("Device I/O error: %d\n", error);
		return -1;
	}

	if (stats.fEof)
	{
		DBG("Device has been detached\n");
		return -1;
	}

	if (stats.cbInQue == 0)
		return 0;

	if (stats.cbInQue < size)
		size = stats.cbInQue;
#else
	fd_set set;
	struct timeval tv = { 0, timeout };

	FD_ZERO(&set);
	FD_SET(device, &set);

	if (0 >= select(device + 1, &set, NULL, NULL, &tv))
		return 0;
#endif

	result = read(device, buffer, size);

	if (result == 0)
	{
		ERR("Serial port has gone\n");
		return -1;
	}

	return result;
}
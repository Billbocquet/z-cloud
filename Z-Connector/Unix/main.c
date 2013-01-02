#include "common.h"
#include <signal.h>
#include <argtable2.h>
#include "common.h"
#include "log.h"
#include "ioutil.h"
#include "serial.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "ssl.h"

// termination flag
int IsTerminating = 0;

#ifdef WIN32
HANDLE hStoppingEvent;
#endif

#define HAS_STR(s) ((s != NULL && strlen(s) > 0) ? 1 : 0)

const char *hex_letters = "0123456789ABCDEF";

void DumpBuffer(const char *data, int length)
{
	char line[49];
	int i, j;

	line[48] = '\0';

	DBG("%d bytes:\n", length);

	for (i = 0; i < length; i++)
	{
		line[3 * (i % 16) + 0] = hex_letters[(data[i] & 0xF0) >> 4];
		line[3 * (i % 16) + 1] = hex_letters[(data[i] & 0x0F)];
		line[3 * (i % 16) + 2] = ' ';

		if ((i % 16) == 15)
			DBG(" %s\n", line);
	}

	if ((length % 16) != 0)
	{
		line[3 * (length % 16)] = '\0';
		DBG(" %s\n", line);
	}
}

int isRunning()
{
#ifdef WIN32
	if (WAIT_OBJECT_0 == WaitForSingleObject(hStoppingEvent, 1))
		return 0;
#endif

	if (IsTerminating == 0)
		return 1;

	return 0;
}

int worker(int dev, SOCKET sock, SSL *sslSession, const char *certFile, const char *keyFile, const char *caCertFile)
{
	int bytes;
	char buffer[1024];

	INFO("Loop started\n");

	while (isRunning())
	{
		bytes = ReadPortTimeout(dev, buffer, sizeof(buffer), 1000);
		if (bytes == -1) break;

		if (bytes > 0)
		{
			DBG("device has data\n");
			DumpBuffer(buffer, bytes);

			if (sslSession != NULL)
				SSL_write(sslSession, buffer, bytes);
			else
				send(sock, buffer, bytes, 0);
			continue;
		}

		bytes = ReadSockTimeout(sock, sslSession, buffer, sizeof(buffer), 1000);
		if (bytes == -1) break;

		if (bytes > 0)
		{
			DBG("socket has data\n");
			DumpBuffer(buffer, bytes);
			write(dev, buffer, bytes);
			continue;
		}

		_sleep(10);
	}

	INFO("Loop ended\n");
	return 0;
}

void sig_handler(int signal)
{
	IsTerminating = 1;
}

#define RETURN(x) \
	SSLCloseSession(sslSession); \
	SSLFreeContext(sslContext); \
	if (sock != SOCKET_ERROR) closesocket(sock); \
	if (dev != -1) close(dev); \
	CloseLog(); \
	return (x)

int main_impl(const char *deviceName, const char *serverName, int port, const char *certFile, const char *keyFile, const char *caCertFile, const char *logFile)
{
	struct hostent *srv;
	int dev = -1;
	FILE* log = NULL;
	SOCKET sock = SOCKET_ERROR;
	struct sockaddr_in srvaddr;
	int keepalive = 1, hasCert, retCode;
	SSL_CTX *sslContext = NULL;
	SSL *sslSession = NULL;
	int sslRead;
	char b[100];

	// initial validation

	if (port < 1 || port > 65535)
	{
		printf("Port must be between 1 and 65535\n");
		return 3;
	}

	srv = gethostbyname(serverName);
	if (srv == NULL)
	{
		srv = gethostbyaddr(serverName, 4, AF_INET);
		if (srv == NULL)
		{
			printf("Cannot resolve server address: %s\n", serverName);
			return 3;
		}
	}

	hasCert = HAS_STR(certFile) + HAS_STR(keyFile) + HAS_STR(caCertFile);
	if (hasCert != 0 && hasCert != 3)
	{
		printf("Either all or none certificates must be specified\n");
		return 3;
	}

	// initial validation complete
	// connecting to server and opening device

	OpenLog(logFile);

	INFO("Opening device...\n");

	dev = open(deviceName, O_BINARY | O_RDWR | O_NOCTTY);
	if (dev == -1)
	{
		ERR("Failed to open device: %s\n", strerror(errno));
		RETURN(3);
	}

	DBG("Configuring device...\n");

	ConfigureDevice(dev);

	DBG("Creating a socket...\n");

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR)
	{
		ERR("Cannot create a socket\n");
		RETURN(3);
	}

	DBG("Enabling keep-alives on the socket...\n");
	if (0 != setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)))
	{
		WARN("Failed to enable keep-alives\n");
	}
	
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons((u_short) port);
	memcpy(&srvaddr.sin_addr, srv->h_addr_list[0], 4);
	
	INFO("Connecting to server...\n");

	if (0 != connect(sock, (const struct sockaddr *) &srvaddr, sizeof(srvaddr)))
	{
		ERR("Cannot connect to %s:%d\n", serverName, port);
		RETURN(3);
	}

	if (0 != hasCert)
	{
		if (0 != SSLInit())
		{
			RETURN(3);
		}

		sslContext = SSLCreateContext(certFile, keyFile, caCertFile);
		if (sslContext == NULL)
		{
			RETURN(3);
		}

		sslSession = SSLHandshake(sock, sslContext);
		if (sslSession == NULL)
		{
			RETURN(3);
		}

		sslRead = SSL_read(sslSession, b, sizeof(b));
		switch (SSL_get_error(sslSession, sslRead))
		{
		case SSL_ERROR_NONE:
			break;

		case SSL_ERROR_ZERO_RETURN:
			DBG("Server has closed the SSL connection\n");

			SSLCloseSession(sslSession);

			sslSession = SSLHandshake(sock, sslContext);
			if (sslSession == NULL)
			{
				RETURN(3);
			}
			break;

		default:
			DEFAULT_HANDLER("Failed to receive server response", sslSession, sslRead);
			RETURN(3);
		}
	}
	
	
	IsTerminating = 0;

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
#ifdef WIN32
	signal(SIGBREAK, sig_handler);
#endif

	retCode = worker(dev, sock, sslSession, certFile, keyFile, caCertFile);

	RETURN(retCode);
}

#pragma region entry point

// ==============================================================================
// entry point of the program - performs initialization and command line parsing
// ==============================================================================

#define CLEAN_RETURN(x) \
	arg_freetable(argTable, sizeof(argTable) / sizeof(argTable[0])); \
	return (x);

#define IS_ARG_SET(x) (x->count > 0)
#define SAFE_INT(x, def) IS_ARG_SET(x) ? x->ival[0] : (def)
#define SAFE_FILE(x, def) IS_ARG_SET(x) ? x->filename[0] : (def)

int main(int argc, char **argv)
{
	const char* progname = "z-connector";

    struct arg_file *device = arg_file1("dD", "device", NULL, "path to the serial port of Z-Wave dongle");
    struct arg_str *server = arg_str1("sS", "server", NULL, "IP address or host name to connect to");
    struct arg_int *port = arg_int0("pP", "port", NULL, "port number (defaults to 9087)");
	struct arg_file *cert = arg_file0(NULL, "cert", NULL, "personal certificate");
	struct arg_file *key = arg_file0(NULL, "key", NULL, "personal certificate key");
	struct arg_file *cacert = arg_file0(NULL, "cacert", NULL, "CA certificate");
	struct arg_file *log = arg_file0("lL", "log", NULL, "file to write log to (defaults to stdout)");
	struct arg_lit *debug = arg_lit0(NULL, "debug", "log debugging information");
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_end *end = arg_end(20);

	void* argTable[] = {device, server, port, cert, key, cacert, log, debug, help, end};

#ifdef WIN32
	WSADATA wsa;
	WORD wsaVersion = MAKEWORD(2, 2);
	int wsaError;
	DWORD win32err;
#endif

	int retCode, nErrors;

	if (arg_nullcheck(argTable) != 0)
	{
        printf("%s: insufficient memory\n", progname);

		CLEAN_RETURN(1);
    }

	nErrors = arg_parse(argc, argv, argTable);

    if (IS_ARG_SET(help))
    {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argTable, "\n");
        //printf("Print certain system information.  With no options, same as -s.\n\n");
        arg_print_glossary(stdout, argTable,"  %-25s %s\n");
        printf("\nReport bugs to <bugs@z-wave.me>.\n");

        CLEAN_RETURN(0);
    }

	LogLevel = (IS_ARG_SET(debug)) ? 3 : 2;

    if (nErrors > 0)
	{
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);

        CLEAN_RETURN(2);
    }

#ifdef WIN32
	hStoppingEvent = CreateEvent(NULL, TRUE, FALSE, "Local\\Z-Agent-Stop");
	if (hStoppingEvent == NULL)
	{
		win32err = GetLastError();
		if (win32err == ERROR_INVALID_HANDLE)
		{
			printf("Another instance is already running\n");

			CLEAN_RETURN(0);
		}
		else
		{
			printf("Failed to create synchronization event: %d\n", win32err);

			CLEAN_RETURN(3);
		}
	}

	wsaError = WSAStartup(wsaVersion, &wsa);
	if (wsaError != 0)
	{
		printf("WSA Startup failed with code %d\n", wsaError);

		CloseHandle(hStoppingEvent);
		CLEAN_RETURN(3);
	}

	if (wsa.wVersion != wsaVersion)
	{
		printf("Required WSA version not found\n");

		CloseHandle(hStoppingEvent);
		CLEAN_RETURN(3);
	}
#endif

	retCode = main_impl(device->filename[0], 
						server->sval[0], SAFE_INT(port, 9087),
						SAFE_FILE(cert, ""), SAFE_FILE(key, ""), SAFE_FILE(cacert, ""), 
						SAFE_FILE(log, "-"));

#ifdef WIN32
	WSACleanup();

	CloseHandle(hStoppingEvent);
#endif

	CLEAN_RETURN(retCode);
}

#pragma endregion

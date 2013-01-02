#include "common.h"
#include "log.h"

// global logging level
int LogLevel = 0;

// log file object
FILE *logFile = NULL;

void OpenLog(const char *fileName)
{
	logFile = (strcmp(fileName, "-") == 0) ? stdout : fopen(fileName, "a");

	if (logFile == NULL)
	{
		printf("Failed to open log file (%s): %s\n", fileName, strerror(errno));
	}
}

void CloseLog()
{
	if (logFile != NULL && logFile != stdout)
		fclose(logFile);
	logFile = NULL;
}

void Log(int level, const char *fmt, ...)
{
	va_list ap;

	if (level > LogLevel) return;

    va_start(ap, fmt);
	vfprintf((logFile != NULL) ? logFile : stdout, fmt, ap);
	va_end(ap);

	fflush((logFile != NULL) ? logFile : stdout);
}

#ifndef _LOG_H_
#define _LOG_H_

extern int LogLevel;

void OpenLog(const char *fileName);
void CloseLog();

void Log(int level, const char *fmt, ...);

#define ERR(s, ...) Log(0, "[E] " s, ##__VA_ARGS__)
#define WARN(s, ...) Log(1, "[W] " s, ##__VA_ARGS__)
#define INFO(s, ...) Log(2, "[I] " s, ##__VA_ARGS__)
#define DBG(s, ...) Log(3, "[D] " s, ##__VA_ARGS__)

#endif
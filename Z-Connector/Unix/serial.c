#include "common.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "serial.h"
#include "log.h"

void ConfigureDevice(int device)
{
#ifdef WIN32
	struct _DCB cfg;
	COMMTIMEOUTS tms;
	HANDLE hFile = (HANDLE)_get_osfhandle(device);

	cfg.DCBlength = sizeof(cfg);

	if (GetCommState(hFile, &cfg))
	{
		cfg.BaudRate = CBR_115200;
		cfg.ByteSize = 8;
		cfg.fParity = NOPARITY;
		cfg.StopBits = ONESTOPBIT;
		cfg.fRtsControl = RTS_CONTROL_DISABLE;
		cfg.fInX = FALSE;
		cfg.fOutX = FALSE;

		if (!SetCommState(hFile, &cfg))
		{
			WARN("Failed to alter device state: %d\n", GetLastError());
		}
	}
	else
	{
		WARN("Failed to get device state: %d\n", GetLastError());
	}
#else
	struct termios cfg;		
	memset(&cfg, 0, sizeof(cfg));
	
	if (0 != tcflush(device, TCIOFLUSH))
	{
		WARN("Failed to flush device: %d\n", errno);
	}
	
	tcgetattr(device, &cfg);
	
	cfg.c_cflag = CS8 | CLOCAL | CREAD;
	cfg.c_iflag = IGNPAR;
	cfg.c_oflag = 0;
	cfg.c_lflag = 0; // ICANON;
	cfg.c_cc[VMIN] = 1;
	cfg.c_cc[VTIME] = 0;
		
	cfsetspeed(&cfg, B115200);
	
	cfmakeraw(&cfg);
 
	if (0 != tcsetattr(device, TCSAFLUSH, &cfg))
	{
		WARN("Failed to alter device settings: %d\n", errno);
	}
#endif
}

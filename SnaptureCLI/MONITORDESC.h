#ifndef __MONITORDESC__
#define __MONITORDESC__

#include <Windows.h>

class MONITORDESC
{
public:
	HMONITOR monitorHandle;
	HDC monitorDC;
	RECT Bounds;
	UINT DPI_X, DPI_Y;
};
#endif
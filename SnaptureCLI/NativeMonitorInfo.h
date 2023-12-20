#pragma once
#include <Windows.h>
#include <vector>
#include <strsafe.h>
#include "MONITORDESC.h"
#include <string>

class nativeMonitorInfo
{
public:
	MONITORDESC* pMonitorDesc;
	std::vector<MONITORDESC*> pMonitors;
	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		nativeMonitorInfo* pThis = reinterpret_cast<nativeMonitorInfo*>(pData);
		pThis->pMonitorDesc = new MONITORDESC;
		pThis->pMonitorDesc->monitorHandle = hMon;
		pThis->pMonitorDesc->monitorDC = hdc;
		pThis->pMonitorDesc->Bounds = *lprcMonitor;
		GetDpiForMonitor(hMon, MDT_EFFECTIVE_DPI, &pThis->pMonitorDesc->DPI_X, &pThis->pMonitorDesc->DPI_Y);
		pThis->pMonitors.push_back(pThis->pMonitorDesc);

		//delete pThis->pMonitorDesc;

		return TRUE;
	}

	nativeMonitorInfo()
	{
		EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
	}
	~nativeMonitorInfo()
	{
		pMonitors.clear();
		
	}
private:
protected:
};

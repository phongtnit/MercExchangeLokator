#pragma once
#include <Windows.h>
#include <vector>
#include <strsafe.h>
#include "MONITORDESC.h"
#include "NativeMonitorInfo.h"
#include "Dpi.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Windows;

namespace com {
	namespace HellStormGames 
	{
		namespace Imaging 
		{
			public ref class MonitorInfo 
			{
			public:
				property IntPtr ^MonitorHandle;
				property IntPtr ^MonitorDC;
				property String^ Name;
				property String^ ModelName;
				property System::Drawing::Rectangle^ ScreenRect;
				property com::HellStormGames::Imaging::Dpi^ Dpi;
			private:
			protected:

			};

			public ref class Monitor 
			{
			public:
				
				System::Collections::Generic::List<MonitorInfo^> ^Monitors;
				Monitor() 
				{

					nMonitorInfo = new nativeMonitorInfo();

					Monitors = gcnew System::Collections::Generic::List<MonitorInfo^>();

					for (int x = 0; x < nMonitorInfo->pMonitors.size(); x++) 
					{
						MonitorInfo^ monitorInfo = gcnew MonitorInfo();
					
						monitorInfo->Dpi = gcnew Dpi;
						monitorInfo->Dpi->X = nMonitorInfo->pMonitors[x]->DPI_X;
						monitorInfo->Dpi->Y = nMonitorInfo->pMonitors[x]->DPI_X;
						monitorInfo->Dpi->ScaleFactor = nMonitorInfo->pMonitors[x]->DPI_X / 96.0f;
					
						monitorInfo->MonitorDC = gcnew IntPtr(nMonitorInfo->pMonitors[x]->monitorDC);
						monitorInfo->MonitorHandle = gcnew IntPtr(nMonitorInfo->pMonitors[x]->monitorHandle);
						int t, l, w, h;
						t = nMonitorInfo->pMonitors[x]->Bounds.top;
						l = nMonitorInfo->pMonitors[x]->Bounds.left;
						w = nMonitorInfo->pMonitors[x]->Bounds.right - l;
						h = nMonitorInfo->pMonitors[x]->Bounds.bottom - t;

						monitorInfo->ScreenRect = gcnew System::Drawing::Rectangle(t, l, w, h);
						
						Monitors->Add(monitorInfo);

						delete monitorInfo;
						delete nMonitorInfo;
					}
				}
			private:
				nativeMonitorInfo* nMonitorInfo;
				
			};
		}
	}
}
#pragma once
#include "DX.h"

public ref class DXCaptureer {
public:
	DXCaptureer() 
	{

	}
	bool Initialize() {
		dxCapturerUnamanged = new DXCapturerUnmanaged();
		return dxCapturerUnamanged->Initialize();
	}
	System::Drawing::Bitmap^ GrabDesktopScreen() {
		System::Drawing::Bitmap^ pBitmap = nullptr;
		HGDIOBJ hGDIObj = dxCapturerUnamanged->CaptureDesktop();
		if (hGDIObj) {
			pBitmap = System::Drawing::Image::FromHbitmap((System::IntPtr)hGDIObj);
			hGDIObj = NULL;
			return pBitmap;
		}
		else
			return nullptr;
	}
	void Release() {
		dxCapturerUnamanged->Release();
		delete dxCapturerUnamanged;
	}
private:
	DXCapturerUnmanaged* dxCapturerUnamanged;
};
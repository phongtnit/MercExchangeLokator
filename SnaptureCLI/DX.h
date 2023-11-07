#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgiformat.h>
#pragma comment(lib, "d3d11.lib")

#define SAFE_RELEASE(x) if(x) { x->Release(); x=NULL; }

#include <Windows.h>
#include <atlbase.h>
#include <memory>

using namespace ATL;

// Driver types supported
D3D_DRIVER_TYPE gDriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE
};
UINT gNumDriverTypes = ARRAYSIZE(gDriverTypes);

// Feature levels supported
D3D_FEATURE_LEVEL gFeatureLevels[] =
{
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_1
};

UINT gNumFeatureLevels = ARRAYSIZE(gFeatureLevels);

class DXCapturerUnmanaged {
public:
	DXCapturerUnmanaged() {

	}
	~DXCapturerUnmanaged() {

	}
	bool Initialize() 
	{
		D3D_FEATURE_LEVEL lFeatureLevel;
		HRESULT hr(E_FAIL);
		bool bInit = false;
		UINT CreationFlags = 0;
#if defined(_DEBUG) 
			CreationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Create device
		for (UINT DriverTypeIndex = 0; DriverTypeIndex < gNumDriverTypes; ++DriverTypeIndex)
		{
			hr = D3D11CreateDevice(
				0,
				gDriverTypes[DriverTypeIndex],
				0,
				0,
				gFeatureLevels,
				gNumFeatureLevels,
				D3D11_SDK_VERSION,
				&pDevice,
				&lFeatureLevel,
				&pDeviceContext);

			if (SUCCEEDED(hr))
			{
				// Device creation success, no need to loop anymore
				bInit = true;
				break;
			}
			SAFE_RELEASE(pDevice);
			SAFE_RELEASE(pDeviceContext);
			bInit = false;
		}
		if (pDevice == NULL)
			return false;

		IDXGIDevice* pDXGIDevice = NULL;
		hr = pDevice->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));
		if (FAILED(hr)) {
			return false;
		}
		IDXGIAdapter* pDXGIAdapter = NULL;
		hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));
		if (FAILED(hr)) 
		{
			SAFE_RELEASE(pDXGIDevice);
			return false;
		}
		SAFE_RELEASE(pDXGIDevice);
		UINT output = 0;

		IDXGIOutput* pDXGIOutput = NULL;
		hr = pDXGIAdapter->EnumOutputs(output, &pDXGIOutput);
		if (FAILED(hr)) 
		{
			SAFE_RELEASE(pDXGIAdapter);
			return false;
		}

		SAFE_RELEASE(pDXGIAdapter);

		IDXGIOutput1* pDXGIOutput1 = NULL;
		hr = pDXGIOutput->QueryInterface(IID_PPV_ARGS(&pDXGIOutput1));
		if (FAILED(hr)) {
			SAFE_RELEASE(pDXGIOutput);
			return false;
		}

		SAFE_RELEASE(pDXGIOutput);

		hr = pDXGIOutput1->DuplicateOutput(pDevice, &pDesktopDuplication);
		if (FAILED(hr)) {
			SAFE_RELEASE(pDXGIOutput1);
			return false;
		}

		SAFE_RELEASE(pDXGIOutput1);

		pDesktopDuplication->GetDesc(&pOutputDuplDesc);
		
		/*
		D3D11_TEXTURE2D_DESC desc = { 0 };

		
		desc.Width = pOutputDuplDesc.ModeDesc.Width;
		desc.Height = pOutputDuplDesc.ModeDesc.Height;
		desc.Format = pOutputDuplDesc.ModeDesc.Format;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.MipLevels = 1;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		hr = pDevice->CreateTexture2D(&desc, 0, &pGDIImage);

		if (FAILED(hr)) {
			SAFE_RELEASE(pDesktopDuplication);
			return false;
		}

		if (pGDIImage == NULL) {
			SAFE_RELEASE(pDesktopDuplication);
			return false;
		}
		*/

		/*
		desc.Width = pOutputDuplDesc.ModeDesc.Width;

		desc.Height = pOutputDuplDesc.ModeDesc.Height;

		desc.Format = pOutputDuplDesc.ModeDesc.Format;

		desc.ArraySize = 1;

		desc.BindFlags = 0;

		desc.MiscFlags = 0;

		desc.SampleDesc.Count = 1;

		desc.SampleDesc.Quality = 0;

		desc.MipLevels = 1;

		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_STAGING;

		hr = pDevice->CreateTexture2D(&desc, NULL, &pDestImage);
		if (FAILED(hr)) {
			SAFE_RELEASE(pGDIImage);
			SAFE_RELEASE(pDesktopDuplication);
			return false;
		}
		*/

		return true;
	}

	HRESULT AcquireNextFrame(UINT timeout, IDXGIResource **resource) 
	{
		HRESULT hr = pDesktopDuplication->AcquireNextFrame(
			timeout,
			&lFrameInfo,
			resource);

		return hr;
	}

	HBITMAP CaptureDesktop()
	{

		IDXGIResource* lDesktopResource = NULL;
		ID3D11Texture2D* pDestImage = NULL;

		HRESULT hr = E_FAIL;

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = pOutputDuplDesc.ModeDesc.Width;
		desc.Height = pOutputDuplDesc.ModeDesc.Height;
		desc.Format = pOutputDuplDesc.ModeDesc.Format;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.MipLevels = 1;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE_STAGING;

		hr = pDevice->CreateTexture2D(&desc, NULL, &pDestImage);
		if (FAILED(hr)) 
		{
			OutputDebugString(L"DX::CaptureDesktop failed to create destination texture2d.\n");
			return NULL;
		}

		hr = AcquireNextFrame(750, &lDesktopResource);
		if (FAILED(hr))
		{
			if ((hr != DXGI_ERROR_ACCESS_LOST) && (hr != DXGI_ERROR_WAIT_TIMEOUT)) {
				OutputDebugString(L"DX::CaptureDesktop -> Failed to acquire frame.\n");
				SAFE_RELEASE(lDesktopResource);
				SAFE_RELEASE(pAcquiredDesktopImage);
				return NULL;
			}
			else if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
				OutputDebugString(L"DX::CaptureDesktop -> DXGI WAIT TIMEOUT.\n");
				SAFE_RELEASE(lDesktopResource);
				SAFE_RELEASE(pAcquiredDesktopImage);
				return NULL;
			}
		}
		
		if (pAcquiredDesktopImage) {
			SAFE_RELEASE(pAcquiredDesktopImage);
		}

		//OutputDebugString(L"DX::AcquireNextFrame returned with SUCCESS.\n");
		hr = lDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pAcquiredDesktopImage);
		
		SAFE_RELEASE(lDesktopResource);

		pDeviceContext->CopyResource(pDestImage, pAcquiredDesktopImage);

		if (pDestImage == nullptr) {
			OutputDebugString(L"DX::CaptureDesktop->Destination Texture2D is NULL.\n");
			SAFE_RELEASE(pAcquiredDesktopImage);
			return NULL;
		}
		
		SAFE_RELEASE(pAcquiredDesktopImage);

		/*
		// Copy image into GDI drawing texture
		pDeviceContext->CopyResource(pGDIImage, pAcquiredDesktopImage);
		
		// Copy image into CPU access texture
		pDeviceContext->CopyResource(pDestImage, pGDIImage);
		SAFE_RELEASE(pGDIImage);
		*/

		/*
		
		// Draw cursor image into GDI drawing texture
		IDXGISurface1* lIDXGISurface1;
		hr = pGDIImage->QueryInterface(IID_PPV_ARGS(&lIDXGISurface1));

		CURSORINFO lCursorInfo = { 0 };

		lCursorInfo.cbSize = sizeof(lCursorInfo);

		auto lBoolres = GetCursorInfo(&lCursorInfo);

		if (lBoolres == TRUE)
		{
			if (lCursorInfo.flags == CURSOR_SHOWING)
			{
				auto lCursorPosition = lCursorInfo.ptScreenPos;

				auto lCursorSize = lCursorInfo.cbSize;

				HDC  lHDC;

				lIDXGISurface1->GetDC(FALSE, &lHDC);

				DrawIconEx(
					lHDC,
					lCursorPosition.x,
					lCursorPosition.y,
					lCursorInfo.hCursor,
					0,
					0,
					0,
					0,
					DI_NORMAL | DI_DEFAULTSIZE);

				lIDXGISurface1->ReleaseDC(NULL);
				lHDC = NULL;
			}

		}
		*/
		//-- causing memory leak.
		HBITMAP gdiTexture = D3D11_CreateHBITMAP(pDestImage);
		
		if (gdiTexture)
		{
			hr = ReleaseFrame();

			if (hr == S_OK) 
			{
				if (pDestImage) {
					pDestImage->Release();
				}
				return gdiTexture;
			}
			else 
			{
				OutputDebugString(L"Unable to release frame.");
				return NULL;
			}
		}
		return NULL;
	}

	HBITMAP D3D11_CreateHBITMAP(_In_ ID3D11Texture2D* src) {

		// Copy from CPU access texture to bitmap buffer
		
		D3D11_TEXTURE2D_DESC desc = { 0 };
		src->GetDesc(&desc);

		D3D11_MAPPED_SUBRESOURCE resource;
		UINT subresource = D3D11CalcSubresource(0, 0, 0);
		pDeviceContext->Map(src, subresource, D3D11_MAP_READ, 0, &resource);
		
		BYTE* pData = reinterpret_cast<BYTE*>(resource.pData);

		HBITMAP hBitmapTexture = CreateCompatibleBitmap(GetDC(NULL), pOutputDuplDesc.ModeDesc.Width, pOutputDuplDesc.ModeDesc.Height);
		SetBitmapBits(hBitmapTexture, pOutputDuplDesc.ModeDesc.Width * pOutputDuplDesc.ModeDesc.Height * 4, pData);

		pDeviceContext->Unmap(src, NULL);
		
		pData = NULL;
		resource = { 0 };

		return hBitmapTexture;
	}

	HRESULT ReleaseFrame()
	{
		HRESULT hr(E_FAIL);
		hr = pDesktopDuplication->ReleaseFrame();
		if (hr != S_OK) {
			OutputDebugString(L"Failed to release frame.\n");
		}
		return hr;
	}
	HGDIOBJ D3D11_CaptureRegion(ID3D11Texture2D** src, UINT sx, UINT sy, UINT source_width, UINT source_height, ID3D11Texture2D** dest) 
	{

		return NULL;
	}
	void Release() 
	{
		SAFE_RELEASE(pAcquiredDesktopImage);
		//SAFE_RELEASE(pDestImage);
		SAFE_RELEASE(pDesktopDuplication);
		SAFE_RELEASE(pDeviceContext);
		SAFE_RELEASE(pDevice);
	}

	ID3D11Texture2D* pAcquiredDesktopImage = 0;
private:
	ID3D11Device* pDevice = NULL;
	ID3D11DeviceContext* pDeviceContext = 0;
	IDXGIOutputDuplication* pDesktopDuplication = 0;
	ID3D11Texture2D* pGDIImage = 0;
	//ID3D11Texture2D* pDestImage = 0;
	DXGI_OUTPUT_DESC pOutputDesc;
	DXGI_OUTDUPL_DESC pOutputDuplDesc;
	
	DXGI_OUTDUPL_FRAME_INFO lFrameInfo;
protected:

};
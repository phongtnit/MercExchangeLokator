#pragma once
#pragma unmanaged 
#include <Windows.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#pragma managed
#include "DXCapturer.h"

using namespace System;
using namespace System::Drawing;

#pragma region Global Declarations

public enum FrameCapturingState {
	IDLE,
	STARTED,
	STOPPED,
	CAPTURING
};

public enum class  FrameCapturingMethod {
	GDI,
	DX
};
#pragma endregion

#pragma region Event Handlers And Arguments

public ref class FrameCapturedEventArgs : EventArgs {
public:
	property System::Drawing::Bitmap^ ScreenCapturedBitmap;
	property int FrameCount;
	FrameCapturedEventArgs(System::Drawing::Bitmap^ bitmap, int FrameCount) {
		this->ScreenCapturedBitmap = bitmap;
		this->FrameCount = FrameCount;
	}
protected:

};
public ref class FrameCapturingEventArgs : EventArgs {
public:
	FrameCapturingEventArgs() 
	{
		
	}
};
public ref class FrameChangedEventArgs : EventArgs {
public:
	FrameChangedEventArgs() {

	}
};

#pragma endregion


namespace com
{
	namespace HellScape
	{
		namespace ScreenCapture
		{
			public ref class Snapture abstract sealed
			{
			public:
				static event EventHandler<FrameCapturedEventArgs^>^ onFrameCaptured;
				static event EventHandler<FrameCapturingEventArgs^>^ onFrameCapturingStarted;
				static event EventHandler<FrameCapturingEventArgs^>^ onFrameCapturingStopped;
				static event EventHandler<FrameChangedEventArgs^>^ onFrameChanged;

				property static double FPS 
				{
					double get() {
						return defaultFPS;
					}
					void set(double value) {
						defaultFPS = value;
					}
				}
				property static bool isActive;
				property static int FrameCount;
				property static FrameCapturingState CapturingState;
				property static int ScreenWidth 
				{
					int get() {
						return GetSystemMetrics(SM_CXSCREEN);
					}
				}
				property static int ScreenHeight {
					int get() {
						return GetSystemMetrics(SM_CYSCREEN);
					}
				}

				static void Start(FrameCapturingMethod captureMethod)
				{
					isActive = true;
					CapturingState = FrameCapturingState::STARTED;
					frameCaptureMethod = captureMethod;
					if (frameCaptureMethod == FrameCapturingMethod::DX) {
						if (!initDXCapturing()) 
						{
							throw gcnew Exception("DX Capturing Initialization failed.");
						}
					}
					onFrameCapturingStarted(NULL, gcnew FrameCapturingEventArgs());

					//BeginCapturing();
				}
				static void CaptureDesktop()
				{
					CaptureDesktopFrame();
				}

				static void CaptureRegion(System::Drawing::Point^ start, System::Drawing::Size^ size) {
					CaptureRegion(start->X, start->Y, start->X + size->Width, start->Y + size->Height);
				}
				static void CaptureRegion(System::Drawing::Rectangle ^ rect) {
					CaptureRegion(rect->X, rect->Y, rect->Width, rect->Height);
				}

				static void CaptureRegion(int x, int y, int width, int height) 
				{
					CapturingState = FrameCapturingState::CAPTURING;
					if (CurrentBitmap)
						CurrentBitmap = nullptr;

					CurrentBitmap = CaptureScreenRegion(x, y, width, height);
					if (CurrentBitmap != nullptr) {
						onFrameCaptured(nullptr, gcnew FrameCapturedEventArgs(CurrentBitmap, FrameCount));
						FrameCount = FrameCount + 1;
					}
				}

				static void Stop() {
					isActive = false;
					CapturingState = FrameCapturingState::STOPPED;
					onFrameCapturingStopped(0, gcnew FrameCapturingEventArgs());
					EndCapturing();
				}
				static void Release() 
				{
					ReleaseCapturer();
				}
			private:
				static int defaultFPS = 30;
				property static System::Drawing::Bitmap^ PreviousBitmap;
				property static System::Drawing::Bitmap^ CurrentBitmap;
				property static FrameCapturingMethod frameCaptureMethod;

				static DXCaptureer^ dxCapturer;

				static bool initDXCapturing() {
					dxCapturer = gcnew DXCaptureer();
					return dxCapturer->Initialize();
				}
				
				static void CaptureScreenRegionFrame(int x, int y, int width, int height) 
				{
					CapturingState = FrameCapturingState::CAPTURING;
					if (CurrentBitmap)
						CurrentBitmap = nullptr;
					
					CurrentBitmap = CaptureScreenRegion(x, y, width, height);

					if (CurrentBitmap != nullptr)
					{
						if (CurrentBitmap != PreviousBitmap)
						{
							//PreviousBitmap = (System::Drawing::Bitmap^)CurrentBitmap->Clone();
						}
						onFrameCaptured(nullptr, gcnew FrameCapturedEventArgs(CurrentBitmap, FrameCount));
						FrameCount = FrameCount + 1;
					}

				}

				static void CaptureDesktopFrame() 
				{
					CapturingState = FrameCapturingState::CAPTURING;
					if (CurrentBitmap)
						CurrentBitmap = nullptr;

					if (frameCaptureMethod == FrameCapturingMethod::DX)
						CurrentBitmap = dxCapturer->GrabDesktopScreen();
					else
						CurrentBitmap = CaptureScreen();

					if (CurrentBitmap != nullptr)
					{
						if (CurrentBitmap != PreviousBitmap)
						{
							//PreviousBitmap = (System::Drawing::Bitmap^)CurrentBitmap->Clone();
						}
						onFrameCaptured(nullptr, gcnew FrameCapturedEventArgs(CurrentBitmap, FrameCount));
						FrameCount = FrameCount + 1;
					}
				}
				
				static void BeginCapturing()
				{
					DateTime^ now = nullptr;
					while (isActive) 
					{
						now = DateTime::Now;
						long currentMS = now->Millisecond;
						long deltaFPS = currentMS - FPS;
						long previousMS = currentMS;
						CapturingState = FrameCapturingState::CAPTURING;
						if (deltaFPS < FPS) 
						{
						}
						else 
						{
							CaptureDesktopFrame();
						}
					}

					delete now;
				}

				static System::Drawing::Bitmap^ CaptureScreen()
				{
					int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
					int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
					
					System::Drawing::Bitmap^ bitmap = nullptr; //gcnew System::Drawing::Bitmap(nScreenWidth, nScreenHeight);

					HWND hDesktopWnd = GetDesktopWindow();
					HDC hDesktopDC = GetDC(hDesktopWnd);
					HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
					HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
						nScreenWidth, nScreenHeight);
					SelectObject(hCaptureDC, hCaptureBitmap);
					BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight,
						hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);

					ReleaseDC(hDesktopWnd, hDesktopDC);
					DeleteDC(hCaptureDC);

					bitmap = System::Drawing::Image::FromHbitmap((IntPtr)hCaptureBitmap);
					DeleteObject(hCaptureBitmap);
					return bitmap;
				}
				
				static System::Drawing::Bitmap^ CaptureScreenRegion(System::Drawing::Point^ start, System::Drawing::Size size)
				{
					return CaptureScreenRegion(start->X, start->Y, start->X + size.Width, start->Y + size.Height);
				}
				static System::Drawing::Bitmap^ CaptureScreenRegion(System::Drawing::Rectangle ^rect) 
				{
					return CaptureScreenRegion(rect->X, rect->Y, rect->Width, rect->Height);
				}

				static System::Drawing::Bitmap^ CaptureScreenRegion(int x, int y, int width, int height) 
				{
					int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
					int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
					
					System::Drawing::Bitmap^ bitmap = nullptr; //gcnew System::Drawing::Bitmap(nScreenWidth, nScreenHeight);

					HWND hDesktopWnd = GetDesktopWindow();
					HDC hDesktopDC = GetDC(hDesktopWnd);
					HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);

					HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
						width, height);

					SelectObject(hCaptureDC, hCaptureBitmap);

					BitBlt(hCaptureDC, 0, 0, width, height,
						hDesktopDC, x, y, SRCCOPY | CAPTUREBLT);

					ReleaseDC(hDesktopWnd, hDesktopDC);
					DeleteDC(hCaptureDC);

					bitmap = System::Drawing::Image::FromHbitmap((IntPtr)hCaptureBitmap);
					DeleteObject(hCaptureBitmap);
					return bitmap;
				}

				static void EndCapturing()
				{
					isActive = false;

					if (PreviousBitmap != nullptr) {
						delete PreviousBitmap;
					}
					if (CurrentBitmap != nullptr) {
						delete CurrentBitmap;
					}

					ReleaseCapturer();

					
				}
				static System::Drawing::Bitmap^ GetCapturedBitmap()
				{
					return nullptr;
				}
				static void ReleaseCapturer() {
					if (dxCapturer) 
					{
						dxCapturer->Release();
						delete dxCapturer;
					}
				}
			protected:

			};


		}
	}
}

#pragma once
#pragma unmanaged 
#include <Windows.h>
#include <ShellScalingApi.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Shcore.lib")

#pragma managed
#include "DXCapturer.h"
#include "MonitorInfor.h"

using namespace System;
using namespace System::Drawing;
using namespace com::HellStormGames::Imaging;

namespace com {
	namespace HellStormGames 
	{
		namespace ScreenCapture 
		{
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
#pragma region Snapture Class
			public ref class Snapture
			{
			public:
				event EventHandler<FrameCapturedEventArgs^>^ onFrameCaptured;
				event EventHandler<FrameCapturingEventArgs^>^ onFrameCapturingStarted;
				event EventHandler<FrameCapturingEventArgs^>^ onFrameCapturingStopped;
				event EventHandler<FrameChangedEventArgs^>^ onFrameChanged;

				property Int32 Resolution 
				{
					Int32 get() {
						return _Resolution;
					}
				private:
					void set(Int32 value) {
						_Resolution = value;
					}
				}
				property  double FPS
				{
					double get() {
						return defaultFPS;
					}
					void set(double value) {
						defaultFPS = value;
					}
				}

				property bool isDPIAware;
				property  bool isActive;
				property  int FrameCount;
				property  FrameCapturingState CapturingState;
				property int ScreenWidth
				{
					int get() 
					{
						return GetSystemMetricsForDpi(SM_CXSCREEN, _monitorInfo->Monitors[CurrentMonitorIndex]->Dpi->X); //GetSystemMetrics(SM_CXSCREEN);
					}
				}
				property int ScreenHeight 
				{
					int get() 
					{
						return GetSystemMetricsForDpi(SM_CYSCREEN, _monitorInfo->Monitors[CurrentMonitorIndex]->Dpi->X);
					}
				}

				property UINT CurrentMonitorIndex;
				
				property Monitor^ MonitorInfo 
				{
					Monitor^ get() 
					{
						return _monitorInfo;
					}
				private:
					void set(Monitor^ value) {
						_monitorInfo = value;
					}
				}
				
				static property Snapture^ Instance {
					Snapture^ get() {
						return _instance;
					}
				}
				Snapture() 
				{
					if (_instance == nullptr)
						_instance = this;

					_monitorInfo = gcnew Monitor();
					CurrentMonitorIndex = 0;
				}
				Snapture(int monitorIndex) 
				{
					if (_instance == nullptr)
						_instance = this;

					_monitorInfo = gcnew Monitor();
					CurrentMonitorIndex = monitorIndex;
					
				}
				~Snapture() 
				{
					_monitorInfo->Monitors->Clear();
					_monitorInfo->Monitors = nullptr;
					delete _monitorInfo;
					delete _instance;
				}

				void SetBitmapResolution(Int32 value) 
				{
					Resolution = value;
				}
				 void Start(FrameCapturingMethod captureMethod)
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

					//-- we should also obtain monitors information as well.
					//BeginCapturing();
				}
				 void CaptureDesktop()
				{
					CaptureDesktopFrame();
				}

				 void CaptureRegion(System::Drawing::Point^ start, System::Drawing::Size^ size) {
					CaptureRegion(start->X, start->Y, start->X + size->Width, start->Y + size->Height);
				}
				 void CaptureRegion(System::Drawing::Rectangle^ rect) {
					CaptureRegion(rect->X, rect->Y, rect->Width, rect->Height);
				}

				 void CaptureRegion(int x, int y, int width, int height)
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

				 void Stop() {
					isActive = false;
					CapturingState = FrameCapturingState::STOPPED;
					onFrameCapturingStopped(0, gcnew FrameCapturingEventArgs());
					EndCapturing();
				}
				 void Release()
				{
					ReleaseCapturer();
				}
			private:
				int defaultFPS = 30;
				Int32 _Resolution = 96;
				property  System::Drawing::Bitmap^ PreviousBitmap;
				property  System::Drawing::Bitmap^ CurrentBitmap;
				property  FrameCapturingMethod frameCaptureMethod;
				property Monitor ^_monitorInfo;
				
				DXCaptureer^ dxCapturer;

				static Snapture^ _instance;

				 bool initDXCapturing() {
					dxCapturer = gcnew DXCaptureer();
					return dxCapturer->Initialize();
				}

				 void CaptureScreenRegionFrame(int x, int y, int width, int height)
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

				 void CaptureDesktopFrame()
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

				 void BeginCapturing()
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

				 System::Drawing::Bitmap^ CaptureScreen()
				{
					int nScreenWidth = ScreenWidth, nScreenHeight = ScreenHeight;
					 
					System::Drawing::Bitmap^ bitmap = nullptr; //gcnew System::Drawing::Bitmap(nScreenWidth, nScreenHeight);

					HWND hDesktopWnd = GetDesktopWindow();
					HDC hDesktopDC =  GetDC(hDesktopWnd);
					HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
					HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,
						nScreenWidth, nScreenHeight);
					SelectObject(hCaptureDC, hCaptureBitmap);

					BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight,
						hDesktopDC, 0, 0, SRCCOPY);

					ReleaseDC(hDesktopWnd, hDesktopDC);
					DeleteDC(hCaptureDC);

					bitmap = System::Drawing::Image::FromHbitmap((IntPtr)hCaptureBitmap);
					bitmap->SetResolution(Resolution, Resolution);
					DeleteObject(hCaptureBitmap);
					return bitmap;
				}

				 System::Drawing::Bitmap^ CaptureScreenRegion(System::Drawing::Point^ start, System::Drawing::Size size)
				{
					return CaptureScreenRegion(start->X, start->Y, start->X + size.Width, start->Y + size.Height);
				}
				 System::Drawing::Bitmap^ CaptureScreenRegion(System::Drawing::Rectangle^ rect)
				{
					return CaptureScreenRegion(rect->X, rect->Y, rect->Width, rect->Height);
				}

				 System::Drawing::Bitmap^ CaptureScreenRegion(int x, int y, int width, int height)
				{
					 int nScreenWidth = ScreenWidth;
					 int nScreenHeight = ScreenHeight;

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
					bitmap->SetResolution(Resolution, Resolution);
					DeleteObject(hCaptureBitmap);
					return bitmap;
				}

				 void EndCapturing()
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
				 System::Drawing::Bitmap^ GetCapturedBitmap()
				{
					return nullptr;
				}
				 void ReleaseCapturer() 
				 {
					if (dxCapturer)
					{
						dxCapturer->Release();
						delete dxCapturer;
					}

				}
			protected:

			};

#pragma endregion

		}
	}
}

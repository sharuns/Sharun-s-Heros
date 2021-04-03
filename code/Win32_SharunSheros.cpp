/*
Date : 12th feb 2021
Author : Sharun S
Place : Chennai , India 
*/

#include < windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

//for better readability purposes
#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int16_t int16;
typedef int32_t bool32;
typedef int32_t int32;
typedef int64_t int64;


typedef  float real32;
typedef  double real64; //IEEE 754 spec, for representation of a float in binary



//cleaned up global variables
struct Win32_offscreen_buffer{

 BITMAPINFO BitmapInfo;
 void *BitmapMemory;
 int Width;
 int Height;
 int Pitch;
 int BytesPerPixel;
};

struct Win32_Window_dimension{

	int Height;
	int Width;
};

struct Win32_sound_output{

int SamplesPerSecond = 48000;
int ToneHz = 256;
int ToneVolume = 200;
uint32 RunningSampleIndex = 0;
int WavePeriod = SamplesPerSecond/ToneHz;
int HalfWavePeriod = WavePeriod/2;
int BytesPerSample = sizeof(int16_t)*2;
int SecondaryBufferSize = SamplesPerSecond*BytesPerSample;
};


global_variable bool Global_Running; //temporary way to close the window 
global_variable Win32_offscreen_buffer Global_BackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

//Trying to link to library using function pointer ourselves rather than hardcoding at compile time
//Get the function declarations from header files

//Note : XInputGetState handling
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE (x_input_get_state);
//Stub function to prevent crash, does not do anything useful
X_INPUT_GET_STATE(XInputGetStateStub){

	return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state * XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

//Note : XInputSetState handling
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
//Stub function to prevent crash, does not do anything useful
X_INPUT_SET_STATE(XInputSetStateStub){

	return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state * XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


//Steps that window loader does on loading our Program to load libraries
internal void Win32LoadXInput(void){

	//Need Diagnostic here
	HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
	if(!XInputLibrary){
		HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
	}
	if(XInputLibrary){

		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState"); 

	}
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


internal void 
Win32InitDSound(HWND Window, int32 SamplePerSecond,  int32 BufferSize){

	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if(DSoundLibrary){


		direct_sound_create *DirectSoundCreate = ( direct_sound_create*)
			GetProcAddress(DSoundLibrary, "DirectSoundCreate");

		LPDIRECTSOUND DirectSound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0,&DirectSound,0))){
			WAVEFORMATEX WaveFormat = {};

			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = 2;
			WaveFormat.nSamplesPerSec = SamplePerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/8;
			WaveFormat.nAvgBytesPerSec = (WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign);
			WaveFormat.cbSize = 0;

			if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))){

				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;


				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer,0))){

					if(SUCCEEDED( PrimaryBuffer->SetFormat(&WaveFormat))){
						OutputDebugStringA("Created Primary Buffer\n");
					}else{

					}//if(SUCCEEDED( PrimaryBuffer...
				}else{


				}//if(SUCCEEDED(CreateSoundBu

			}else{


			}//if(Succeeded(DirectSound...)
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = 0;
				BufferDescription.dwBufferBytes = BufferSize;
				BufferDescription.lpwfxFormat= &WaveFormat;
				LPDIRECTSOUNDBUFFER SecondaryBuffer;
				HRESULT Error = (DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer,0));
				if(SUCCEEDED(Error)){


				}else{


				}	//if(SUCCEEDED(DirectSound->Creat	
					
		}else{


		}//if (DirectSound...)


	}else{


	}//if(DSoundLibrary)
}

internal
Win32_Window_dimension Win32GetWindowDimension(HWND Window){
	
	Win32_Window_dimension Result;
	RECT ClientRect; //The writable section in window excluding the - [] X marks at top
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}


internal void RenderGrdaient(Win32_offscreen_buffer Buffer,int BlueOffset, int GreenOffset){

	uint8 * Row = (uint8 *)Buffer.BitmapMemory;

	for(int y = 0 ; y < Buffer.Height ; ++y){

		uint32 *Pixel = (uint32 *) Row; //LITTLE ENDIAN - Reason for blue bb gg rr
		for(int x = 0 ; x < Buffer.Width; ++x){

				uint8 Blue = (x + BlueOffset);
				uint8 Green = (y + GreenOffset);
				uint8 Red = (x + BlueOffset); //Gives a Pink tint

				*Pixel++ = ((Green << 8) | Blue | (Red << 16));
		}
		Row += Buffer.Pitch;
	}

}

internal void
Win32ResizeDIBSection(Win32_offscreen_buffer  *Buffer, int Width , int Height){
	//DIB section is windows term for Rendering something that user sends ?
	
/*	if(BitMapHandle){
		DeleteObject(BitMapHandle);
	}
	if(!BitmapDeviceContext){
		BitmapDeviceContext =  CreateCompatibleDC(0);
	}
*/	
	if(Buffer->BitmapMemory){

		VirtualFree(Buffer->BitmapMemory,0,MEM_RELEASE);
	}

//	BITMAPINFO BitmapInfo;

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
	Buffer->BitmapInfo.bmiHeader.biWidth =  Buffer->Width;
	//Negative sign here indicates to windows that the Buffer's first 3 bytes is the
	//top left pixel
	Buffer->BitmapInfo.bmiHeader.biHeight = -Buffer->Height;
	Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
	Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
	Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	

		/*BitMapHandle = CreateDIBSection(
		BitmapDeviceContext, &BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0,0
		);


		ReleaseDC(0,BitmapDeviceContext);*/

	int BitmapMemorySize = (Buffer->Width * Buffer->Height)*Buffer->BytesPerPixel;
	Buffer->BitmapMemory = VirtualAlloc(0,BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	//RenderGrdaient(128,0);
	Buffer->Pitch = Width*Buffer->BytesPerPixel;

	
}
//Optimized by removing pointer for RECT
internal void
Win32DisplayWindow(HDC DeviceContext,int WindowWidth,int WindowHeight,
					Win32_offscreen_buffer Buffer,
				  	int X,int Y, int Width, int Height){
//Dirty rectangle update	
	StretchDIBits(DeviceContext,
								/*X,Y,Width,Height,
								X,Y,Width,Height,*/
								0,0, WindowWidth, WindowHeight,
								0,0, Buffer.Width, Buffer.Height,
								Buffer.BitmapMemory,
								&Buffer.BitmapInfo,
								DIB_RGB_COLORS,
								SRCCOPY);

}




//This is the callback that windows
LRESULT CALLBACK MainWindowCallback(
		HWND   Window,
		UINT   Message,
		WPARAM WParam,
		LPARAM LParam
) {
	LRESULT Result = 0;
	
	switch(Message)
	{
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE\n");
		}
		break;
		case WM_DESTROY:
		{
			//to recreate window...more like a error happened
			Global_Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		}
		break;
		case WM_CLOSE:
		{
			//PostQuitMessage(0);
			//Post a message pop up for user
			Global_Running = false;
			OutputDebugStringA("WM_CLOSE\n");	
		}
		break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");	
		}
		break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{

			uint32 VKCode = WParam;
			bool WasDown = ((LParam & (1 << 30)) != 0 );
			bool IsDown = ((LParam & (1 << 31)) == 0);

			if(WasDown != IsDown){
				if(VKCode == 'W'){

				}
				else if(VKCode == 'A')
				{

				}
				else if(VKCode == 'S')
				{
					
				}
				else if(VKCode == 'D')
				{
					
				}
				else if(VKCode == 'Q')
				{
					
				}
				else if(VKCode == 'E')
				{
					OutputDebugStringA("WM_ACTIVATEAPP\n");	
				}
				else if(VKCode == VK_UP)
				{
					OutputDebugStringA("Up\n");	
				}
				else if(VKCode == VK_DOWN)
				{
					OutputDebugStringA("Down\n");	
				}
				else if(VKCode == VK_LEFT)
				{
					OutputDebugStringA("Left\n");	
				}
				else if(VKCode == VK_RIGHT)
				{
					OutputDebugStringA("Right\n");	
				}else if(VKCode == VK_ESCAPE)
				{
					
				}
				else if( VKCode == VK_SPACE)
				{
					
				}
			}
			bool32 AltKeyDown =  ((LParam & (1 << 29)));
			if(VKCode == VK_F4){
				Global_Running = false;
			}
		}
		break;
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window,&Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			Win32_Window_dimension Dimension =  Win32GetWindowDimension(Window);
			Win32DisplayWindow(DeviceContext,Dimension.Width,Dimension.Height,
								Global_BackBuffer, X,Y,Width,Height);
			EndPaint(Window,&Paint);
		}
		break;
		default :
		{
		//	OutputDebugStringA("default\n");
			Result = DefWindowProc(Window,Message,WParam,LParam);
		}
		break;
	}
		
	return (Result);
}

internal void Win32FillSoundBuffer(Win32_sound_output * SoundOutput, DWORD BytesToLock, DWORD BytesToWrite){

	VOID * Region1;
	DWORD Region1Size;
	VOID * Region2;
	DWORD Region2Size;
	if(SUCCEEDED( GlobalSecondaryBuffer->Lock(
									BytesToLock,
									BytesToWrite,
									&Region1,&Region1Size,
									&Region2,&Region2Size,
									0)))
		{
			int16_t *SampleOut = (int16_t *)Region1;
			DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
								
			//Assert can be used Region1Size
			for(DWORD SampleIndex = 0 ; SampleIndex < Region1SampleCount ; ++SampleIndex ){
									
				real32 t = 2.0f*Pi32*(real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod; 
				real32  SineValue = sin(t);
				int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
				//int16_t SampleValue = ((RunningSampleIndex++/HalfSquareWavePeriod)%2) ? ToneVolume: -ToneVolume;
				*SampleOut++ = SampleValue;
				*SampleOut++ = SampleValue;
				++SoundOutput->RunningSampleIndex;
									
			}
								
			DWORD Region2SampleCpount = Region2Size/SoundOutput->BytesPerSample;
			SampleOut = (int16_t *)Region2;
			for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCpount ; ++SampleIndex){
				//int16_t SampleValue = ((RunningSampleIndex++/HalfSquareWavePeriod)%2) ? ToneVolume: -ToneVolume;
				real32 t = 2.0f*Pi32*(real32)SoundOutput->RunningSampleIndex / (real32)SoundOutput->WavePeriod; 
				real32  SineValue = sin(t);
				int16_t SampleValue = (int16_t)(SineValue * SoundOutput->ToneVolume);
				*SampleOut++ = SampleValue;
				*SampleOut++ = SampleValue;
				++SoundOutput->RunningSampleIndex;
			}

			GlobalSecondaryBuffer->Unlock(Region1,Region1Size,
										  Region2,Region2Size);
		}
}
extern "C"{
//Entry point for windows
	int CALLBACK WinMain(
	  HINSTANCE Instance,
	  HINSTANCE PrevInstance,
	  LPSTR     CommandLine,
	  int       ShowCode)
	{


		LARGE_INTEGER PerfCountFrequencyResult;
		QueryPerformanceFrequency(&PerfCountFrequencyResult);
		int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
		//Better Practice , when need to intialize an entire struct to 0
		Win32LoadXInput();
		WNDCLASS windowClass = {0}; //initializer;

		Win32ResizeDIBSection(&Global_BackBuffer, 1280,720);

		windowClass.style = CS_HREDRAW | CS_VREDRAW;//These flags tell that it needs to re-paint the entire window
		windowClass.lpfnWndProc = MainWindowCallback; //registering callback method
		windowClass.hInstance = Instance; 
		windowClass.lpszClassName = "SharunSHeroWindowsClass";


		//register window class for opening the window

		if(RegisterClass(&windowClass)){
			HWND Window =
			 CreateWindowExA( //ASCI version for Windows can use without 'A'
					  0,
					  windowClass.lpszClassName,
					  "SharunS Hero",
					  WS_OVERLAPPEDWINDOW|WS_VISIBLE,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  0,
					  0,
					  Instance,
					  0);

			 if(Window){

			 	Global_Running = true;
			 	
			 	int XOffset = 0 ;
			 	int YOffset = 0 ;


			Win32_sound_output SoundOutput;

			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.ToneHz = 256;
			SoundOutput.ToneVolume = 3000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
			SoundOutput.HalfWavePeriod = SoundOutput.WavePeriod/2;
			SoundOutput.BytesPerSample = sizeof(int16_t)*2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;

			 	Win32InitDSound(Window,SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			 	Win32FillSoundBuffer(&SoundOutput,0,SoundOutput.SecondaryBufferSize);
			 	GlobalSecondaryBuffer->Play(0,0,DSBPLAY_LOOPING);
			
			 	LARGE_INTEGER LastCounter;
			 	QueryPerformanceCounter(&LastCounter);

			 	uint64 LastCycleCount = __rdtsc();
			 	

			 		while (Global_Running){
									 			

							MSG Message;
					 		while(PeekMessage(&Message,0,0,0,PM_REMOVE)){

					 			if(Message.message == WM_QUIT){
					 				Global_Running = false;
					 			}	
					 			TranslateMessage(&Message);
					 			DispatchMessage(&Message);
				 			}//end of while(Peekmessa...)

				 			//TODO : Poll more frequently
				 			for(DWORD ControllerIndex = 0 ;
				 				ControllerIndex < XUSER_MAX_COUNT;
				 				++ControllerIndex){

				 					XINPUT_STATE ControllerState;
				 					if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS){
				 						//Controller is plugged in
				 						XINPUT_GAMEPAD * Pad = &ControllerState.Gamepad;

				 						bool Up 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
				 						bool Down 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
				 						bool Left  	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
				 						bool Right 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
				 						bool Start   = (Pad->wButtons & XINPUT_GAMEPAD_START);
				 						bool Back    = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
				 						bool LeftShoulder  = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				 						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				 						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
				 						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
				 						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
				 						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

				 						int16 StickX = Pad->sThumbLX;
				 						int16 StickY = Pad->sThumbLY;

				 						if(AButton){

				 							YOffset +=3;
				 							
				 						}

				 					}else{

				 						//Controller not available
				 					}


				 			}

				 		/*MSG Message;
				 		//Replace with Peek Message
				 		//BOOL MessageResult = GetMessageA(&Message,0,0,0);
				 		BOOL MessageResult = 
				 		if(MessageResult > 0){
				 			TranslateMessage(&Message);
				 			DispatchMessage(&Message);
				 		}
						else{
							break;
				 		}//if WindowHandle*/
				 		XINPUT_VIBRATION Vibration;
				 		Vibration.wLeftMotorSpeed =60000;
				 		Vibration.wRightMotorSpeed =60000;
				 		XInputSetState(0,&Vibration);
						RenderGrdaient(Global_BackBuffer, XOffset,YOffset);

						//Audio test
						DWORD PlayCursor;
						DWORD WriteCursor;
						//DirectSound output test
						if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition( &PlayCursor,
														&WriteCursor))){

						
						DWORD BytesToWrite;
						DWORD BytesToLock = ( SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
					    if(BytesToLock > PlayCursor){
							 BytesToWrite = (SoundOutput.SecondaryBufferSize - BytesToLock);
							 BytesToWrite += PlayCursor;
						}else{

							BytesToWrite = PlayCursor - BytesToLock;
						}
						
						Win32FillSoundBuffer(&SoundOutput,BytesToLock,BytesToWrite);
					}


						HDC DeviceContext = GetDC(Window);

						Win32_Window_dimension Dimension =  Win32GetWindowDimension(Window);
						Win32DisplayWindow(DeviceContext,Dimension.Width,
										   Dimension.Height,
										   Global_BackBuffer ,
										   0,0,Dimension.Width,Dimension.Height);
						ReleaseDC(Window, DeviceContext);
						++XOffset;


						uint64 EndCycleCount = __rdtsc();	
						LARGE_INTEGER EndCounter;
					 	QueryPerformanceCounter(&EndCounter);


					 	int64 Cyclecount = EndCycleCount - LastCycleCount;	
					 	int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
					 	int32 MSPerFrame = (int32)((1000*CounterElapsed)/PerfCountFrequency);
					 	int32 FPS = PerfCountFrequency/CounterElapsed;
					 	int32 MCPF = (int32)(Cyclecount /(1000 * 1000));
					 	char Buffer[256];	
					 	wsprintf(Buffer,"Msec/Frame : %ds /%d FPS / Cycles %d\n",MSPerFrame,FPS,MCPF);

					 	OutputDebugStringA(Buffer);	
					 	LastCounter = EndCounter;
					 	LastCycleCount = EndCycleCount;

					}//end of while(Global_Running)
			}
			else{

				//TODO : Logging
			}
		}else{

			//Logging
		}
	return 0;
	}//end of Winmain
	
}//end of extern C
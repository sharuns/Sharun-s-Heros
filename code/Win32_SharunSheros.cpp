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

//This technique is called the unity build
// to have only one translation unit
#include "SharunMade.cpp" 
#include "Win32_SharunSheros.h" 

//cleaned up global variables




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


void * PlatformLoadFile(char * FileName){

	return 0;
}

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

internal void 
Win32ClearBuffer(Win32_sound_output * SoundOutput)
{


	VOID * Region1;
	DWORD Region1Size;
	VOID * Region2;
	DWORD Region2Size;
		if(SUCCEEDED( GlobalSecondaryBuffer->Lock(
									0,
									SoundOutput->SecondaryBufferSize,
									&Region1,&Region1Size,
									&Region2,&Region2Size,
									0)))
		{
			uint8 *DestSample = (uint8 *)Region1;	
			for(DWORD ByteIndex = 0 ; ByteIndex < Region1Size ; ++ByteIndex ){
				*DestSample++ = 0;
			}

			DestSample = (uint8 *)Region2;	
			for(DWORD ByteIndex = 0 ; ByteIndex < Region2Size ; ++ByteIndex ){
				*DestSample++ = 0;
			}

			GlobalSecondaryBuffer->Unlock(Region1,Region1Size,
										  Region2,Region2Size);

		}

}

internal 
void Win32FillSoundBuffer(Win32_sound_output * SoundOutput, DWORD BytesToLock, DWORD BytesToWrite,
							game_sound_output_buffer * SourceBuffer)
{

	VOID * Region1;
	DWORD Region1Size;
	VOID * Region2;
	DWORD Region2Size;
	if(SUCCEEDED( GlobalSecondaryBuffer->Lock(
									BytesToLock,BytesToWrite,
									&Region1,&Region1Size,
									&Region2,&Region2Size,
									0)))
		{
			
			DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
			int16_t *DestSample = (int16_t *)Region1;
			int16_t *SourceSample = SourceBuffer->Samples;				
			//Assert can be used Region1Size
			for(DWORD SampleIndex = 0 ;
				 SampleIndex < Region1SampleCount ;
				  ++SampleIndex ){
									
				
				*DestSample++ = *SourceSample++;
				*DestSample++ = *SourceSample++;
				++SoundOutput->RunningSampleIndex;
									
			}
								
			DWORD Region2SampleCpount = Region2Size/SoundOutput->BytesPerSample;
			DestSample = (int16_t *)Region2;
			for(DWORD SampleIndex = 0; 
				SampleIndex < Region2SampleCpount ;
				 ++SampleIndex){
				
				*DestSample++ = *SourceSample++;
				*DestSample++ = *SourceSample++;
				++SoundOutput->RunningSampleIndex;
			}

			GlobalSecondaryBuffer->Unlock(Region1,Region1Size,
										  Region2,Region2Size);
		}
}

internal void
Win32ProcessXInputDigitalButton(DWORD XinputButtonState,
								game_button_state *OldState, DWORD ButtonBit,
								game_button_state *NewState)
{

	
	NewState->EndedDown = ((XinputButtonState & ButtonBit) == ButtonBit);
	NewState->HalftransitionCount = (OldState->EndedDown != NewState->EndedDown)? 1 : 0 ; 

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
				Win32_sound_output SoundOutput = {};

				SoundOutput.SamplesPerSecond = 48000;
				//SoundOutput.RunningSampleIndex = 0;
				//SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond/SoundOutput.ToneHz;
				//SoundOutput.HalfWavePeriod = SoundOutput.WavePeriod/2;
				SoundOutput.BytesPerSample = sizeof(int16_t)*2;
				SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
				SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond/15;
				Win32InitDSound(Window,SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
				Win32ClearBuffer(&SoundOutput);
				GlobalSecondaryBuffer->Play(0,0,DSBPLAY_LOOPING);
			
				int16_t * Samples = (int16 *)VirtualAlloc(0,SoundOutput.SecondaryBufferSize,
				 											MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

#if SHARUN_INTERNAL
		LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);
#else
		LPVOID BaseAddress = 0;
#endif
				game_memory GameMemory = {};
				GameMemory.PermanentStorageSize = Megabytes(64);
				GameMemory.TransientStorageSize = Gigabytes((uint64)4);

				uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
				GameMemory.PermanentStorage = VirtualAlloc(BaseAddress,TotalSize,
				 											MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
			
				GameMemory.TransientStorage = ((uint8 * )GameMemory.PermanentStorage + 
													GameMemory.PermanentStorageSize); 
				 							


				if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage )
				{

					game_input Input[2] = {};
					game_input * NewInput = &Input[0];
					game_input * OldInput = &Input[1];

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



				 			int MaxControllerCount = XUSER_MAX_COUNT;
				 			if(MaxControllerCount > ArrayCount(NewInput->Controllers)){

				 				MaxControllerCount = ArrayCount(NewInput->Controllers);
				 			}
				 			//TODO : Poll more frequently
				 			for(DWORD ControllerIndex = 0 ;
				 				ControllerIndex < MaxControllerCount;
				 				++ControllerIndex){


				 					game_controller_input * OldController = &(OldInput->Controllers[ControllerIndex]);
				 					game_controller_input * NewController = &(NewInput->Controllers[ControllerIndex]);

				 					XINPUT_STATE ControllerState;
				 					if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS){
				 						//Controller is plugged in
				 						XINPUT_GAMEPAD * Pad = &ControllerState.Gamepad;

				 						bool Up 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
				 						bool Down 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
				 						bool Left  	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
				 						bool Right 	 = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
				 						
				 						NewController->IsAnalog = true;
				 						NewController->StartX = OldController->EndX;
				 						NewController->StartY = OldController->EndY;

				 						real32 X;
				 						if(Pad->sThumbLX < 0){

				 							X = (real32)Pad->sThumbLX /32768.0f;
				 						}
				 						else{

				 						X = (real32)Pad->sThumbLX /32767.0f;	
				 						}


				 						NewController->MinX = NewController->MaxX = NewController->EndX = X;

				 						real32 Y;
				 						if(Pad->sThumbLY < 0){

				 							Y = (real32)Pad->sThumbLY /32768.0f;
				 						}
				 						else{

				 						Y = (real32)Pad->sThumbLY /32767.0f;	
				 						}

				 						NewController->MinY = NewController->MaxY = NewController->EndY = Y;

				 						


										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->Down, XINPUT_GAMEPAD_A, 
																		&NewController->Down);
										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->Right, XINPUT_GAMEPAD_B, 
																		&NewController->Right);
										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->Left, XINPUT_GAMEPAD_X, 
																		&NewController->Left);
										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->Up, XINPUT_GAMEPAD_Y, 
																		&NewController->Up);
										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, 
																		&NewController->LeftShoulder);
										Win32ProcessXInputDigitalButton(Pad->wButtons,
																		&OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, 
																		&NewController->RightShoulder);

				 						
										/*
				 						bool Start   = (Pad->wButtons & XINPUT_GAMEPAD_START);
				 						bool Back    = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
				 						bool LeftShoulder  = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				 						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				 						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
				 						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
				 						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
				 						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);
				 						*/

				 					}else{

				 						//Controller not available
				 						//Controller not available
				 					}


				 			}

				 		XINPUT_VIBRATION Vibration;
				 		Vibration.wLeftMotorSpeed =60000;
				 		Vibration.wRightMotorSpeed =60000;
				 		XInputSetState(0,&Vibration);
				 		
				 		
						DWORD BytesToLock  = 0;
						DWORD TargetCursor = 0;
						DWORD BytesToWrite = 0;
						DWORD PlayCursor = 0;
						DWORD WriteCursor =0;
						bool SoundIsValid = false;
						//DirectSound output test
						
						if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition( &PlayCursor,
														&WriteCursor)))
						{

							BytesToLock = ( SoundOutput.RunningSampleIndex * 
										SoundOutput.BytesPerSample) % 
										SoundOutput.SecondaryBufferSize;
					   		 TargetCursor = ((PlayCursor *
					    				(SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) %
					    				SoundOutput.SecondaryBufferSize);

						    if(BytesToLock > PlayCursor){
								 BytesToWrite = (SoundOutput.SecondaryBufferSize - BytesToLock);
								 BytesToWrite += PlayCursor;
							}else{

								BytesToWrite = PlayCursor - BytesToLock;
							}

							SoundIsValid = true;
						}

				 		
				 		game_sound_output_buffer SoundBuffer = {};
				 		SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
				 		SoundBuffer.SampleCount = BytesToWrite/SoundOutput.BytesPerSample;
				 		SoundBuffer.Samples = Samples;


				 		game_offscreen_buffer Buffer = {};
				 		Buffer.Memory = Global_BackBuffer.BitmapMemory ;
				 		Buffer.Width = Global_BackBuffer.Width;
				 		Buffer.Height = Global_BackBuffer.Height;
				 		Buffer.Pitch = Global_BackBuffer.Pitch;

				 		GameUpdateAndRenderer(&GameMemory, NewInput ,&Buffer,&SoundBuffer);
						
						if(SoundIsValid){
							Win32FillSoundBuffer(&SoundOutput,BytesToLock,BytesToWrite,&SoundBuffer);
						}


						HDC DeviceContext = GetDC(Window);

						Win32_Window_dimension Dimension =  Win32GetWindowDimension(Window);
						Win32DisplayWindow(DeviceContext,Dimension.Width,
										   Dimension.Height,
										   Global_BackBuffer ,
										   0,0,Dimension.Width,Dimension.Height);
						ReleaseDC(Window, DeviceContext);


						uint64 EndCycleCount = __rdtsc();	
						LARGE_INTEGER EndCounter;
					 	QueryPerformanceCounter(&EndCounter);

					 	int64 Cyclecount = EndCycleCount - LastCycleCount;	
					 	int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
					 	int32 MSPerFrame = (int32)((1000*CounterElapsed)/PerfCountFrequency);
					 	int32 FPS = PerfCountFrequency/CounterElapsed;
					 	int32 MCPF = (int32)(Cyclecount /(1000 * 1000));
					 	#if 0
					 	char Buffer[256];	
					 	wsprintf(Buffer,"Msec/Frame : %ds /%d FPS / Cycles %d\n",MSPerFrame,FPS,MCPF);

					 	OutputDebugStringA(Buffer);	
					 	#endif
					 	LastCounter = EndCounter;
					 	LastCycleCount = EndCycleCount;


					 	game_input * temp = NewInput;
					 	NewInput = OldInput;
					 	OldInput = temp;

					}
				} //mem check
		}//Global Running
			else{

				//TODO : Logging
			}
		}else{

			//Logging
		}
	return 0;
	}//end of Winmain
	
}//end of extern C
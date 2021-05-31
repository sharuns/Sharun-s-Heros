/*
Date : 12th feb 2021
Author : Sharun S
Place : Chennai , India 
*/

#include < windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>




//This technique is called the unity build
// to have only one translation unit
#include "SharunMade.h" 
#include "Win32_SharunSheros.h" 

//cleaned up global variables




global_variable bool Global_Running; //temporary way to close the window 
global_variable Win32_offscreen_buffer Global_BackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalPerfCountFrequency;

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

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{

	if(Memory){

		VirtualFree(Memory, 0, MEM_RELEASE);
	}
	return;
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
	debug_read_file_result Result = {};
	
	HANDLE FileHandle = CreateFileA(Filename,GENERIC_READ,
								   FILE_SHARE_READ,0,OPEN_EXISTING,
								   0,0);
	if(FileHandle!= INVALID_HANDLE_VALUE){

		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle,&FileSize)){

			uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
			Result.Contents = VirtualAlloc(0,FileSize.QuadPart,MEM_RESERVE|MEM_COMMIT ,PAGE_READWRITE);
			if(Result.Contents){ 	

					DWORD BytesRead;
					if(ReadFile(FileHandle, Result.Contents, FileSize32,&BytesRead,0) && 
						(FileSize32 == BytesRead)){

							Result.ContentSize = FileSize32; // 

					}else{

						DEBUGPlatformFreeFileMemory(Result.Contents);
						Result.Contents = 0 ;
					}
			}

		}
		CloseHandle(FileHandle);
	}

	return (Result);

}




#if 0

void * DEBUGPlatformReadfromFreeMemory(void * Memory)
{
		
}
#endif


 DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
	bool32 Result = false;
	HANDLE FileHandle = CreateFileA(Filename,GENERIC_WRITE,
								   0,0,CREATE_ALWAYS,
								   0,0);
	if(FileHandle!= INVALID_HANDLE_VALUE){

		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory,MemorySize,&BytesWritten,0)){

			Result = (BytesWritten == MemorySize);

		}else{


		}
		CloseHandle(FileHandle);
	}else{

		//Log
	}

	return Result;
}

inline FILETIME 
Win32GetLastWriteTime(char* FileName) {
	
	FILETIME LastWriteTime = {};

	WIN32_FIND_DATA FindData;
	HANDLE FileHandle = FindFirstFileA(FileName, &FindData);

	if (FileHandle != INVALID_HANDLE_VALUE) {
		
		LastWriteTime = FindData.ftLastWriteTime;
		FindClose(FileHandle);
	}

	return (LastWriteTime);
}

internal win32_game_code
Win32LoadGameCode(char * SourceDLLName, char * TempDLLName){

	win32_game_code Result= {};

	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

	//char* TempDLLName = "SharunMade_temp.dll";

	CopyFile(SourceDLLName, TempDLLName,FALSE);

	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	if(Result.GameCodeDLL){

		Result.GetSoundSamples = (game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples"); 
		Result.UpdateAndRenderer = (game_update_and_renderer *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRenderer");  

		Result.IsValid = (Result.UpdateAndRenderer && Result.GetSoundSamples);
	}

	if(!Result.IsValid){

		Result.UpdateAndRenderer = GameUpdateAndRendererStub;
		Result.GetSoundSamples = GameGetSoundSamplesStub;
	}

	return Result;
}/*
 internal win32_game_code
Win32LoadGameCode(char * SourceDLLName, char* TempDLLName){

	win32_game_code Result= {};

	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

	CopyFile(SourceDLLName, TempDLLName,FALSE);

	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	if(Result.GameCodeDLL){

		Result.GetSoundSamples = (game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples"); 
		Result.UpdateAndRenderer = (game_update_and_renderer *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRenderer");  

		Result.IsValid = (Result.UpdateAndRenderer && Result.GetSoundSamples);
	}

	if(!Result.IsValid){

		Result.UpdateAndRenderer = GameUpdateAndRendererStub;
		Result.GetSoundSamples = GameGetSoundSamplesStub;
	}

	return Result;
}
 
 */


internal void 
Win32UnloadGameCode(win32_game_code * GameCode) {

	if (GameCode->GameCodeDLL) {
		
		FreeLibrary(GameCode->GameCodeDLL);
	}

	GameCode->IsValid = false;
	GameCode->UpdateAndRenderer = GameUpdateAndRendererStub;
	GameCode->GetSoundSamples = GameGetSoundSamplesStub;

}

//Steps that window loader does on loading our Program to load libraries
internal void Win32LoadXInput(void){

	//Need Diagnostic here
	HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
	if(!XInputLibrary){
		 XInputLibrary = LoadLibrary("xinput1_3.dll");
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
				//LPDIRECTSOUNDBUFFER SecondaryBuffer;
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

		/*case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
		}
		break;*/
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
Win32ProcessKeyboardMessage( game_button_state *NewState,
							 bool32 IsDown)
{

	
	NewState->EndedDown = IsDown;
	++NewState->HalftransitionCount; 

}


internal void
Win32ProcessXInputDigitalButton(DWORD XinputButtonState,
								game_button_state *OldState, DWORD ButtonBit,
								game_button_state *NewState)
{

	
	NewState->EndedDown = ((XinputButtonState & ButtonBit) == ButtonBit);
	NewState->HalftransitionCount = (OldState->EndedDown != NewState->EndedDown)? 1 : 0 ; 

}

internal real32 
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold){

	real32 Result = 0;

	if(Value < DeadZoneThreshold){

		Result = (real32)Value/32768.0f;
	}
	else if (Value > DeadZoneThreshold){
		Result = (real32)Value /32767.0f;	
	}
	return Result;
}	

internal void
Win32ProcessPendingMessages(game_controller_input * KeyboardController){

MSG Message;

while(PeekMessage(&Message,0,0,0,PM_REMOVE)){

					 	

	switch(Message.message){

		case WM_QUIT:
		{
			Global_Running = false;

		}break;
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = (uint32)Message.wParam;
			bool WasDown = ((Message.lParam & (1 << 30)) != 0 );
			bool IsDown= ((Message.lParam & (1 << 31)) == 0);

			if(WasDown != IsDown){
				if(VKCode == 'W'){
					OutputDebugStringA("W\n");	
					Win32ProcessKeyboardMessage( &KeyboardController->MoveUp,IsDown);	
				}
				else if(VKCode == 'A')
				{
						
					Win32ProcessKeyboardMessage( &KeyboardController->MoveLeft,IsDown);
				}
				else if(VKCode == 'S')
				{
					Win32ProcessKeyboardMessage( &KeyboardController->MoveDown,IsDown);
				}	
				else if(VKCode == 'D')
				{
					Win32ProcessKeyboardMessage( &KeyboardController->MoveRight,IsDown);
				}
				else if(VKCode == 'Q')
				{
					Win32ProcessKeyboardMessage( &KeyboardController->LeftShoulder,IsDown);
				}
				else if(VKCode == 'E')
				{
					Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder,IsDown);
				}
				else if(VKCode == VK_UP)
				{
					Win32ProcessKeyboardMessage(&KeyboardController->ActionUp,IsDown);
				}
				else if(VKCode == VK_DOWN)
				{
					
					Win32ProcessKeyboardMessage(&KeyboardController->ActionDown,IsDown);
				}
				else if(VKCode == VK_LEFT)
				{
					Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft,IsDown);
				}
				else if(VKCode == VK_RIGHT)
				{
					Win32ProcessKeyboardMessage(&KeyboardController->ActionRight,IsDown);	
				}
				else if(VKCode == VK_ESCAPE)
				{
					Global_Running = false;	
				}
				else if( VKCode == VK_SPACE)
				{
				}
				}
				bool32 AltKeyDown =  ((Message.lParam & (1 << 29)));
				if((VKCode == VK_F4)&&(AltKeyDown)){
					Global_Running = false;
				}

				}break;
		default:
			{					 			
					TranslateMessage(&Message);
					DispatchMessage(&Message);					 				
			}break;
		}

	}//end of while(Peekmessa...)

}



inline LARGE_INTEGER
Win32GetWallClock(void){
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return (Result);
}


inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End){

	real32 Result = ((real32)(End.QuadPart - Start.QuadPart)/
						(real32)(GlobalPerfCountFrequency));
	return (Result);
}


internal void
Win32DebugDrawVertical(Win32_offscreen_buffer * global_BackBuffer, int X, int Top,
						 int Bottom, uint32 Color){


	uint8 * Pixel = ((uint8 * )global_BackBuffer->BitmapMemory + 
					X * global_BackBuffer->BytesPerPixel +
					Top * global_BackBuffer->Pitch );
	for(int Y = Top;
		Y < Bottom;
		++Y){

		*(uint32*)Pixel = Color;
		Pixel += global_BackBuffer->Pitch;
	}
}

internal void 
Win32DrawSoundBufferMarker(Win32_offscreen_buffer* BackBuffer, Win32_sound_output* soundOutput,
							real32 C,int PadX, int Top, int Bottom, DWORD Value, DWORD Color){

	real32 XReal32 = (C * (real32)Value);
	int X = PadX + (int)XReal32;
	Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);

}

internal void 
Win32DebugSyncDisplay(Win32_offscreen_buffer * BackBuffer,
	int MarkerCount, win32_debug_time_marker * Markers,
	int CurrentMarkerIndex,
	Win32_sound_output  * soundOutput,  real32 TargertSecondPerFrame){

	int PadX = 16;
	int PadY = 16;

	int LineHeight = 64;
	real32 C= (real32)(BackBuffer->Width -2*PadX)/ (real32)soundOutput->SecondaryBufferSize;
	  
	for(int MarkerIndex = 0 ;
		MarkerIndex < MarkerCount;
		++MarkerIndex){

		DWORD PlayColor = 0xFFFFFFFF;
		DWORD WriteColor = 0xFFFF0000;
		int Top = PadY;
		int Bottom = PadY + LineHeight;
		if (MarkerIndex == CurrentMarkerIndex) {
		
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
		}

		win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
		Win32DrawSoundBufferMarker(BackBuffer, soundOutput, C,PadX, Top, Bottom, ThisMarker->PlayCursor, PlayColor);
		Win32DrawSoundBufferMarker(BackBuffer, soundOutput,C, PadX, Top, Bottom, ThisMarker->WriteCursor, WriteColor);
		
		
	}


}	

internal void
CatStrings(size_t SourceCountA, char* SourceA,
	size_t SourceCountB, char* SourceB,
	size_t DestCount, char* Dest) {
	
	for (int Index = 0;
		Index < SourceCountA;
		++Index) {
		
		*Dest++ = *SourceA++;
	}

	for (int Index = 0;
		Index < SourceCountB;
		++Index) {

		*Dest++ = *SourceB++;
	}

	*Dest++ = 0;

}

extern "C"{
//Entry point for windows
	int CALLBACK WinMain(
	  HINSTANCE Instance,
	  HINSTANCE PrevInstance,
	  LPSTR     CommandLine,
	  int       ShowCode)
	{
		char EXEFileName[MAX_PATH];
		DWORD SizeofFileName = GetModuleFileName(0, EXEFileName, sizeof(EXEFileName));
		char* OnePastLastSlash = EXEFileName;
		OutputDebugStringA("========================================\n");
		OutputDebugStringA(EXEFileName);
		OutputDebugStringA("\n");
		for (char* Scan = EXEFileName; *Scan; ++Scan) {
			if (*Scan == '\\') {
				OnePastLastSlash = Scan + 1;
			}
		}
		OutputDebugStringA(OnePastLastSlash);
		OutputDebugStringA("\n");

		char SourceGameCodeDLLFilename[] = "SharunMade.dll";
		char SourceGameCodeDLLFullPath[MAX_PATH];
		CatStrings(OnePastLastSlash - EXEFileName, EXEFileName,
			sizeof(SourceGameCodeDLLFilename) - 1, SourceGameCodeDLLFilename,
			sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

		char TempGameCodeDLLFilename[] = "SharunMade_temp.dll";
		char TempGameCodeDLLFullPath[MAX_PATH];
		CatStrings(OnePastLastSlash - EXEFileName, EXEFileName,
			sizeof(TempGameCodeDLLFilename) - 1, TempGameCodeDLLFilename,
			sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

		OutputDebugStringA(SourceGameCodeDLLFullPath);
		OutputDebugStringA("\n");
		OutputDebugStringA(TempGameCodeDLLFullPath );
		OutputDebugStringA("\n");
		OutputDebugStringA("========================================\n");
		

		LARGE_INTEGER PerfCountFrequencyResult;
		QueryPerformanceFrequency(&PerfCountFrequencyResult);
		int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

		//Making Scherdular granuality to 1 ms
		//For better Sleep
		UINT DesiredSchedulerMS = 1;	
		bool32 SleepGranular  = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

#define MonitorRefreshHz 60 //used for locking into 30FPS
#define GameUpdateHz  (MonitorRefreshHz/2)


		real32 TargertSecondPerFrame = 1.0f/(real32)GameUpdateHz;//MonitorRefreshHz;

		#define FrameOfAudioLatency 4

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

			if (Window) {

				
				Win32_sound_output SoundOutput = {};

				SoundOutput.SamplesPerSecond = 48000;
				SoundOutput.BytesPerSample = sizeof(int16_t) * 2;
				SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
				SoundOutput.LatencySampleCount = (3 * (SoundOutput.SamplesPerSecond / (GameUpdateHz)));
				SoundOutput.SafetyBytes = ( (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample)/GameUpdateHz)/2;
				Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
				Win32ClearBuffer(&SoundOutput);
				GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);


				Global_Running = true;

				int16_t* Samples = (int16*)VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
					MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if SHARUN_INTERNAL
				LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);
#else
				LPVOID BaseAddress = 0;
#endif
				game_memory GameMemory = {};
				GameMemory.PermanentStorageSize = Megabytes(64);
				GameMemory.TransientStorageSize = Gigabytes((uint64)4); //Try making this as 1 
				GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
				GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
				GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;

				uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
				GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize,
					MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

				GameMemory.TransientStorage = ((uint8*)GameMemory.PermanentStorage +
					GameMemory.PermanentStorageSize);



				if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
				{

					game_input Input[2] = {};
					game_input* NewInput = &Input[0];
					game_input* OldInput = &Input[1];

					LARGE_INTEGER LastCounter = Win32GetWallClock();

					int DebugTimeMarkerIndex = 0;
					win32_debug_time_marker DebugTimeMarkers[GameUpdateHz / 2] = { 0 };
					
					DWORD AudioLatencyBytes = 0;
					real32 AudioLatencySeconds = 0;
					bool32 SoundIsValid = false;
					//char* SourceDLLName = "SharunMade.dll";
					win32_game_code  Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);

					uint64 LastCycleCount = __rdtsc();


					while (Global_Running) {

						FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
						if (CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0) {

							Win32UnloadGameCode(&Game);
							Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath);
						}

						game_controller_input* OldKeyboardController = GetController(OldInput, 0);
						game_controller_input* NewKeyboardController = GetController(NewInput, 0);
						game_controller_input zeroController = {};
						*NewKeyboardController = zeroController;
						NewKeyboardController->IsConnected = true;

						for (int ButtonIndex = 0;
							ButtonIndex < ArrayCount(NewKeyboardController->Button);
							++ButtonIndex) {

							NewKeyboardController->Button[ButtonIndex].EndedDown =
								OldKeyboardController->Button[ButtonIndex].EndedDown;
						}

						Win32ProcessPendingMessages(NewKeyboardController);


						DWORD MaxControllerCount = XUSER_MAX_COUNT;
						if (MaxControllerCount > ArrayCount(NewInput->Controllers) - 1) {

							MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
						}
						//TODO : Poll more frequently
						for (DWORD ControllerIndex = 0;
							ControllerIndex < MaxControllerCount;
							++ControllerIndex) {

							DWORD OurControllerIndex = ControllerIndex + 1;
							game_controller_input* OldController = GetController(OldInput, OurControllerIndex);
							game_controller_input* NewController = GetController(NewInput, OurControllerIndex);

							XINPUT_STATE ControllerState;
							if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
								//Controller is plugged in
								NewController->IsConnected = true;
								XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

								NewController->IsAnalog = true;
								NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX,
									XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
								NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY,
									XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

								if ((NewController->StickAverageX != 0.0f) ||
									(NewController->StickAverageY != 0.0f)) {

									NewController->IsAnalog = true;
								}
								if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
									NewController->StickAverageY = 1.0f;
									NewController->IsAnalog = false;
								}
								if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
									NewController->StickAverageY = -1.0f;
									NewController->IsAnalog = false;
								}

								if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
									NewController->StickAverageX = -1.0f;
									NewController->IsAnalog = false;
								}
								if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
									NewController->StickAverageX = 1.0f;
									NewController->IsAnalog = false;
								}


								real32 Threshold = 0.5f;
								Win32ProcessXInputDigitalButton((NewController->StickAverageX < -Threshold) ? 1 : 0,
									&OldController->MoveLeft, 1,
									&NewController->MoveLeft);
								Win32ProcessXInputDigitalButton((NewController->StickAverageX > Threshold) ? 1 : 0,
									&OldController->MoveRight, 1,
									&NewController->MoveRight);
								Win32ProcessXInputDigitalButton((NewController->StickAverageY < -Threshold) ? 1 : 0,
									&OldController->MoveDown, 1,
									&NewController->MoveDown);
								Win32ProcessXInputDigitalButton((NewController->StickAverageY > Threshold) ? 1 : 0,
									&OldController->MoveUp, 1,
									&NewController->MoveUp);



								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->ActionDown, XINPUT_GAMEPAD_A,
									&NewController->ActionDown);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->ActionRight, XINPUT_GAMEPAD_B,
									&NewController->ActionRight);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->ActionLeft, XINPUT_GAMEPAD_X,
									&NewController->ActionLeft);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->ActionUp, XINPUT_GAMEPAD_Y,
									&NewController->ActionUp);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
									&NewController->LeftShoulder);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
									&NewController->RightShoulder);

								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->Start, XINPUT_GAMEPAD_START,
									&NewController->Start);
								Win32ProcessXInputDigitalButton(Pad->wButtons,
									&OldController->Back, XINPUT_GAMEPAD_BACK,
									&NewController->Back);


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

							}
							else {

								//Controller not available
								//Controller not available
								NewController->IsConnected = false;
							}


						}
						game_offscreen_buffer Buffer = {};
						Buffer.Memory = Global_BackBuffer.BitmapMemory;
						Buffer.Width = Global_BackBuffer.Width;
						Buffer.Height = Global_BackBuffer.Height;
						Buffer.Pitch = Global_BackBuffer.Pitch;

						Game.UpdateAndRenderer(&GameMemory, NewInput, &Buffer);


						DWORD PlayCursor;
						DWORD WriteCursor;

						if (GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK) {
							if (!SoundIsValid) {
								SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
								SoundIsValid = true;
							}
 
							DWORD BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
								SoundOutput.SecondaryBufferSize;

							

							//Check if the audio card is having latency 
							DWORD ExpectedSoundBytesPerFrame = (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample) / GameUpdateHz;
							
							DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedSoundBytesPerFrame;

							DWORD SafeWriteCursor = WriteCursor;
							if (SafeWriteCursor < PlayCursor) {
								SafeWriteCursor += SoundOutput.SecondaryBufferSize;
							}
							SafeWriteCursor += SoundOutput.SafetyBytes;
							bool32 AudioCardIsLowLatency = (SafeWriteCursor  < ExpectedFrameBoundaryByte);

							DWORD TargetCursor = 0;
							
							if (AudioCardIsLowLatency) {

								TargetCursor = ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame;
							}
							else {	
								
								TargetCursor = WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes;
							}
							
							TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;

							DWORD BytesToWrite = 0;
							if (BytesToLock > TargetCursor) {

								BytesToWrite = (SoundOutput.SecondaryBufferSize - BytesToLock);
								BytesToWrite += TargetCursor;
							}
							else {

								BytesToWrite = TargetCursor - BytesToLock;
							}

							game_sound_output_buffer SoundBuffer = {};
							SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
							SoundBuffer.SampleCount = (int16)((int)(BytesToWrite / SoundOutput.BytesPerSample));
							SoundBuffer.Samples = Samples;
							Game.GetSoundSamples(&GameMemory, &SoundBuffer);

#if SHARUN_INTERNAL

							//DWORD PlayCursor;
							//DWORD WriteCursor;
							GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor);

							DWORD UnWrappedWriteCursor = WriteCursor;
							if (UnWrappedWriteCursor < PlayCursor) {
								UnWrappedWriteCursor += SoundOutput.SecondaryBufferSize;
							}

							AudioLatencyBytes = UnWrappedWriteCursor - PlayCursor;
							AudioLatencySeconds = (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
								(real32)SoundOutput.SamplesPerSecond);
#endif
							Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite, &SoundBuffer);
						}
						else {
						
							SoundIsValid = false;
							
						}

						LARGE_INTEGER WorkCounter = Win32GetWallClock();
						real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

						real32 SecondsElapsedForFrame = WorkSecondsElapsed;
						if (SecondsElapsedForFrame < TargertSecondPerFrame) {
							if (SleepGranular) {
								DWORD SleepMS = (DWORD)(1000.0f * (TargertSecondPerFrame - SecondsElapsedForFrame));
								if (SleepMS > 0) {

									Sleep(SleepMS);
								}
							}

							while (SecondsElapsedForFrame < TargertSecondPerFrame) {

								SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
									Win32GetWallClock());

							}
						}
						else {

							//Missed Frame Rate
							//Logging
						}
						LARGE_INTEGER EndCounter = Win32GetWallClock();
						real32 MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
						LastCounter = EndCounter;

						HDC DeviceContext = GetDC(Window);

						Win32_Window_dimension Dimension = Win32GetWindowDimension(Window);
#ifdef SHARUN_INTERNAL
						Win32DebugSyncDisplay(&Global_BackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers,
							DebugTimeMarkerIndex - 1,
							&SoundOutput, TargertSecondPerFrame);
#endif

						 

						Win32DisplayWindow(DeviceContext, Dimension.Width,
							Dimension.Height,
							Global_BackBuffer,
							0, 0, Dimension.Width, Dimension.Height);
						ReleaseDC(Window, DeviceContext);
						    
#ifdef SHARUN_INTERNAL
				//Debug code
				{
					//DWORD PlayCursor;
					//DWORD WriteCursor;
					if ((GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)) == DS_OK) {

						win32_debug_time_marker* Marker = &DebugTimeMarkers[DebugTimeMarkerIndex++];
						if (DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers)) {
							DebugTimeMarkerIndex = 0;
						}
						Marker->PlayCursor = PlayCursor;
						Marker->WriteCursor = WriteCursor;
					}
				}	
#endif
					 	game_input * temp = NewInput;
					 	NewInput = OldInput;
					 	OldInput = temp;




					 	uint64 EndCycleCount = __rdtsc();
					 	uint64 CyclesElapsed = EndCycleCount - LastCycleCount;	
					 	LastCycleCount = EndCycleCount;

					 	real64 FPS = 0.0f;
					 	real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));

					 	char FPSBuffer[256];	
					 	wsprintf(FPSBuffer,"%f ms/f, %f f/s, %f mc/f \n",MSPerFrame,FPS,MCPF);

					 	OutputDebugStringA(FPSBuffer);

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
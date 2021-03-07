/*
Date : 12th feb 2021
Author : Sharun S
Place : Chennai , India 
*/

#include < windows.h>
#include <stdint.h>

//for better readability purposes
#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

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


global_variable bool Global_Running; //temporary way to close the window 
global_variable Win32_offscreen_buffer Global_BackBuffer;




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
	Buffer->BitmapMemory = VirtualAlloc(0,BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

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

extern "C"{
//Entry point for windows
	int CALLBACK WinMain(
	  HINSTANCE Instance,
	  HINSTANCE PrevInstance,
	  LPSTR     CommandLine,
	  int       ShowCode)
	{
		//Better Practice , when need to intialize an entire struct to 0

		WNDCLASS windowClass = {0}; //initializer

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


			 		while (Global_Running){

							MSG Message;
					 		while(PeekMessage(&Message,0,0,0,PM_REMOVE)){

					 			if(Message.message == WM_QUIT){
					 				Global_Running = false;
					 			}	
					 			TranslateMessage(&Message);
					 			DispatchMessage(&Message);
				 			}//end of while(Peekmessa...)

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
						RenderGrdaient(Global_BackBuffer, XOffset,YOffset);
						HDC DeviceContext = GetDC(Window);

						Win32_Window_dimension Dimension =  Win32GetWindowDimension(Window);
						Win32DisplayWindow(DeviceContext,Dimension.Width,
										   Dimension.Height,
										   Global_BackBuffer ,
										   0,0,Dimension.Width,Dimension.Height);
						ReleaseDC(Window, DeviceContext);
						++XOffset;++YOffset;

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
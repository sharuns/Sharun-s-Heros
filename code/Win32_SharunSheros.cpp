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


global_variable bool Running; //temporary way to close the window 

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
//global_variable HBITMAP BitMapHandle;
//global_variable HDC BitmapDeviceContext;

internal void RenderGrdaient(int XOffset, int YOffset){

	int Width = BitmapWidth;
	int Height = BitmapHeight;
	int BytesPerPixel = 4;
	int Pitch = Width * BytesPerPixel;
	uint8 * Row = (uint8 *)BitmapMemory;
	for(int y = 0 ; y < BitmapHeight ; ++y){
		//LITTLE ENDIAN - Reason for blue
		uint32 *Pixel = (uint32 *) Row;
		for(int x = 0 ; x < BitmapWidth ; ++x){
				uint8 Blue = (x + XOffset);
				uint8 Green = (y + YOffset);
				uint8 Red = (x + XOffset);

				*Pixel++ = ((Green << 8) | Blue | (Red << 16));

		}

		Row += Pitch;
	}

}

internal void
Win32ResizeDIBSection(int Width , int Height){
	//DIB section is windows term for Rendering something that user sends ?
	
/*	if(BitMapHandle){
		DeleteObject(BitMapHandle);
	}
	if(!BitmapDeviceContext){
		BitmapDeviceContext =  CreateCompatibleDC(0);
	}
*/	
	if(BitmapMemory){

		VirtualFree(BitmapMemory,0,MEM_RELEASE);
	}

//	BITMAPINFO BitmapInfo;

	BitmapWidth = Width;
	BitmapHeight = Height;
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth =  BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	

		/*BitMapHandle = CreateDIBSection(
		BitmapDeviceContext, &BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0,0
		);


		ReleaseDC(0,BitmapDeviceContext);*/


	int BytesPerPixel = 4;
	int BitmapMemorySize = (Width * Height)*BytesPerPixel;
	BitmapMemory = VirtualAlloc(0,BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	//RenderGrdaient(128,0);


}
//Optimized by removing pointer for RECT
internal void
Win32UpdateWindow(HDC DeviceContext, RECT WindowRect,int X,int Y, int Width, int Height){
//Dirty rectangle update	
	
	int WindowWidth = WindowRect.right - WindowRect.left;
	int WindowHeight = WindowRect.bottom - WindowRect.top;
	StretchDIBits(DeviceContext,
								/*X,Y,Width,Height,
								X,Y,Width,Height,*/
								0,0, BitmapWidth, BitmapHeight,
								0,0, BitmapWidth, BitmapHeight,
								BitmapMemory,
								&BitmapInfo,
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
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSection(Width,Height);
			OutputDebugStringA("WM_SIZE\n");
		}
		break;
		case WM_DESTROY:
		{
			//to recreate window...more like a error happened
			Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		}
		break;
		case WM_CLOSE:
		{
			//PostQuitMessage(0);
			//Post a message pop up for user
			Running = false;
			OutputDebugStringA("WM_CLOSE\n");	
		}
		break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");	
		}
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window,&Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);

			Win32UpdateWindow(DeviceContext,ClientRect,X,Y,Width,Height);
			EndPaint(Window,&Paint);
		}
		break;
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

		windowClass.style = CS_HREDRAW | CS_VREDRAW;//These flags tell that it needs to re-paint the entire window
		windowClass.lpfnWndProc = MainWindowCallback; //registering callback method
		windowClass.hInstance = Instance; 
		windowClass.lpszClassName = "HandmadeHeroWindowsClass";

		//register window class for opening the window

		if(RegisterClass(&windowClass)){
			HWND Window =
			 CreateWindowExA( //ASCI version for Windows can use without 'A'
					  0,
					  windowClass.lpszClassName,
					  "Handmade Hero",
					  WS_OVERLAPPEDWINDOW|WS_VISIBLE,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  CW_USEDEFAULT,
					  0,0,Instance,0);

			 if(Window){

			 	Running = true;
			 	
			 	int XOffset = 0 ;
			 	int YOffset = 0 ;


			 		while (Running){

							MSG Message;
					 		while(PeekMessage(&Message,0,0,0,PM_REMOVE)){

					 			if(Message.message == WM_QUIT){
					 				Running = false;
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
						RenderGrdaient(XOffset,YOffset);
						HDC DeviceContext = GetDC(Window);

						RECT ClientRect;
						GetClientRect(Window, &ClientRect);
						int WindowWidth = ClientRect.right - ClientRect.left;
						int WindowHeight = ClientRect.bottom - ClientRect.top;
						Win32UpdateWindow(DeviceContext,ClientRect,0,0,WindowWidth,WindowHeight);
						ReleaseDC(Window, DeviceContext);
						++XOffset;++YOffset;

					}//end of while(Running)
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
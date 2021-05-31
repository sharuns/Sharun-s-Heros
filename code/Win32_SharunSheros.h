#pragma once

struct Win32_Window_dimension{

	int Height;
	int Width;
};

struct Win32_sound_output{

int SamplesPerSecond = 48000;
uint32 RunningSampleIndex = 0;
int BytesPerSample = sizeof(int16_t)*2;
int LatencySampleCount = SamplesPerSecond/15;
int SecondaryBufferSize = SamplesPerSecond*BytesPerSample;
DWORD SafetyBytes;

};

struct Win32_offscreen_buffer{

 BITMAPINFO BitmapInfo;
 void *BitmapMemory;
 int Width;
 int Height;
 int Pitch;
 int BytesPerPixel;
};

struct win32_debug_time_marker {

	DWORD PlayCursor;
	DWORD WriteCursor;
};


struct  win32_game_code {

	HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
	game_update_and_renderer* UpdateAndRenderer;
	game_get_sound_samples* GetSoundSamples;
	bool32 IsValid;
};



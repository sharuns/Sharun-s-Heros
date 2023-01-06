/*
Date : 12th feb 2021
Author : Sharun S
Place : Chennai , India
*/
#pragma once

#define GAME_SCREEN_RESOLUTION_WIDTH 960
#define GAME_SCREEN_RESOLUTION_HEIGHT 540

struct Win32_Window_dimension{

	int Height;
	int Width;
};

struct Win32_sound_output{

int SamplesPerSecond = 48000;
uint32 RunningSampleIndex = 0;
int BytesPerSample = sizeof(int16_t)*2;
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

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH

struct win32_replay_buffer {

	char ReplayFileName[WIN32_STATE_FILE_NAME_COUNT];
	void* MemoryBlock;
};

struct win32_state {

	uint64 TotalSize;
	void * GameMemoryBlock;
	win32_replay_buffer ReplayBuffers[4];

	HANDLE RecordingHandle;
	int InputRecordingIndex;

	HANDLE PlayBackHandle;
	int InputPlayingIndex;


	char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
	char* OnePastLastEXEFileNameSlash;

};
struct win32_recorded_input {
	int InputCount;
	game_input* InputStream;
};



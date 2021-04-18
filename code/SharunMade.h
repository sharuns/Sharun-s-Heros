#pragma once

/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

//Services that the game provide to the platform layer
/*
	controller input
	bitmap buffer
	soundbuffer //this maybe moved to a thread
	timing
*/

//Defined in the build.bat file
//As assert is performance effecting so we use it only when we define the macro 



#if SHARUN_DEBUG_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

inline uint32
SafeTruncateUInt64(uint64 Value){

	Assert(Value <=0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return Result;

}

//Services that the platform layer provides to the game

#if SHARUN_INTERNAL
struct debug_read_file_result{

	uint32 ContentSize;
	void * Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char * Filename);
internal void *DEBUGPlatformFreeFileMemory(void * Memory);
internal void * DEBUGPlatformReadfromFreeMemory(void * Memory);
internal bool32 DEBUGPlatformWriteEntireFile(char * Filename, uint32 MemorySize, void * Memory);
#endif


struct game_offscreen_buffer{

 //BITMAPINFO BitmapInfo;
 void *Memory;
 int Width;
 int Height;
 int Pitch;
 int BytesPerPixel;
};

struct game_sound_output_buffer{

	int SamplesPerSecond;
	int16 SampleCount;
	int16 * Samples;

};

struct game_button_state{

	int HalftransitionCount;
	bool32 EndedDown;

};

struct game_controller_input{

	bool32 IsAnalog;

	real32 StartX;
	real32 StartY;

	real32 MinX;
	real32 MinY;

	real32 MaxX;
	real32 MaxY;

	real32 EndX;
	real32 EndY;

	union{

		game_button_state Button[6];
		struct{

			game_button_state Up;
			game_button_state Down;
			game_button_state Left;
			game_button_state Right;
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
		};
	};
};

struct game_input{

	game_controller_input Controllers[4];

};

struct game_state{

	int ToneHz;
	int GreenOffset;
	int BlueOffset;

};

struct game_memory{

	bool32 IsInitialized;



	uint64 PermanentStorageSize;
	void * PermanentStorage;

	uint64 TransientStorageSize;
	void * TransientStorage;

};


void GameUpdateAndRenderer(game_memory * Memory,
						   game_input * Input,
						   game_offscreen_buffer * Buffer,
						   game_sound_output_buffer * SoundBuffer);

//Service that the platform layer provides the game
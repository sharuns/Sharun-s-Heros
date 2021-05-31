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

#include <math.h>
#include <stdint.h>

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


#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char * Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);
 
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void * Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

//void * DEBUGPlatformReadfromFreeMemory(void * Memory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(char * Filename, uint32 MemorySize, void * Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);
  	 
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

	bool32 IsConnected;
	bool32 IsAnalog;

	real32 StickAverageX;
	real32 StickAverageY;

	union{

		game_button_state Button[12];
		struct{

			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;
			
			game_button_state ActionUp;
			game_button_state ActionDown;
			game_button_state ActionLeft;
			game_button_state ActionRight;

			game_button_state LeftShoulder;
			game_button_state RightShoulder;

			game_button_state Start;
			game_button_state Back;			

		};
	};
};

struct game_input{

	game_controller_input Controllers[5];

};

inline game_controller_input* GetController(game_input * Input, int ControllerIndex){

	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input * Result =  &Input->Controllers[ControllerIndex];
	return (Result);
}

struct game_state{

	int ToneHz;
	int GreenOffset;
	int BlueOffset;

	real32 tSine;

};

struct game_memory{

	bool32 IsInitialized;

	uint64 PermanentStorageSize;
	void * PermanentStorage;

	uint64 TransientStorageSize;
	void * TransientStorage;

	debug_platform_read_entire_file * DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *  DEBUGPlatformWriteEntireFile;
	debug_platform_free_file_memory * DEBUGPlatformFreeFileMemory;   
	//pass pointers to the debug services


};

#define GAME_UPDATE_AND_RENDERER(name) void name(game_memory * Memory,game_input * Input,game_offscreen_buffer * Buffer)
typedef GAME_UPDATE_AND_RENDERER(game_update_and_renderer);
//Stub function to prevent crash, does not do anything useful
GAME_UPDATE_AND_RENDERER(GameUpdateAndRendererStub){

}

#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory* Memory,game_sound_output_buffer* SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
//Stub function to prevent crash, does not do anything useful
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub){
	
}

//Service that the platform layer provides the game
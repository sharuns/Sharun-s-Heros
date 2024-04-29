/*
Date : 5th Jan 2023
Author : Sharun S
Place : Alleppey , India
*/
#pragma once

//Services that the game provide to the platform layer
/*
	controller input
	bitmap buffer
	soundbuffer //this maybe moved to a thread
	timing
*/

//Defined in the build.bat file
//As assert is performance effecting so we use it only when we define the macro 

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include<intrin.h>
#endif

#include <stdint.h>
#include<float.h>

//for better readability purposes
#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f
#define NUMSCREENS 2000
//#define RoundingFraction 0.5f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int16_t int16;
typedef int32_t bool32;
typedef int32_t int32;
typedef int64_t int64;

typedef size_t memory_index;


typedef  float real32;
typedef  double real64; //IEEE 754 spec, for representation of a float in binary

#define Real32Maximum FLT_MAX

#if SHARUN_DEBUG_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");

#define Kilobytes(Value) ((Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

inline uint32
SafeTruncateUInt64(uint64 Value) {

	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return Result;

}

struct thread_context {

	int PlaceHolder;
};

//Services that the platform layer provides to the game

#if SHARUN_INTERNAL
struct debug_read_file_result {

	uint32 ContentSize;
	void* Contents;
};


#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread,void * Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread,char * Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread,char * Filename, uint32 MemorySize, void * Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

#define BITMAP_BYTES_PER_PIXEL 4 

struct game_offscreen_buffer {

	//BITMAPINFO BitmapInfo;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer {

	int SamplesPerSecond;
	int16 SampleCount;
	int16* Samples;

};

struct game_button_state {

	int HalftransitionCount;
	bool32 EndedDown;

};

struct game_controller_input {

	bool32 IsConnected;
	bool32 IsAnalog;

	real32 StickAverageX;
	real32 StickAverageY;

	union {

		game_button_state Button[12];
		struct {

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

struct game_input {

	game_button_state MouseButton[5];
	int32 MouseX, MouseY, MouseZ;
	real32 dtForFrame;
	game_controller_input Controllers[5];

};

inline game_controller_input* GetController(game_input* Input, int ControllerIndex) {

	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input* Result = &Input->Controllers[ControllerIndex];
	return (Result);
}


struct game_memory {

	bool32 IsInitialized;

	uint64 PermanentStorageSize;
	void* PermanentStorage;

	uint64 TransientStorageSize;
	void* TransientStorage;

	debug_platform_free_file_memory* DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file* DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file* DEBUGPlatformWriteEntireFile;

	//pass pointers to the debug services


};



#define GAME_UPDATE_AND_RENDERER(name) void name(thread_context *Thread,game_memory * Memory,game_input * Input,game_offscreen_buffer * Buffer)
typedef GAME_UPDATE_AND_RENDERER(game_update_and_renderer);
//Stub function to prevent crash, does not do anything useful
GAME_UPDATE_AND_RENDERER(GameUpdateAndRendererStub) {

}

#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context *Thread,  game_memory* Memory,game_sound_output_buffer* SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
//Stub function to prevent crash, does not do anything useful
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub) {

}

//Service that the platform layer provides the game





#define Minimum(A,B) ((A < B) ? (A) : (B))
#define Maximum(A,B) ((A > B) ? (A) : (B))



struct memory_arena {
	memory_index Size;
	uint8* Base;
	memory_index Used;

	int32 TempCount;

};

struct temporary_memory
{
	memory_arena* Arena;
	memory_index Used;

};

inline void
InitailizeArena(memory_arena* Arena, memory_index Size, void * Base) {

	Arena->Size = Size;
	Arena->Base = (uint8 *)Base;
	Arena->Used = 0;
	Arena->TempCount = 0;
}

//Primitive memory allocator for now, which pushed our tiles from previously stored in stack to the 
//dedicated allocated game memory 

#define PushStruct(Arena,type) (type * )PushSize_(Arena, sizeof(type))
#define PushArray(Arena,Count,type) (type*)PushSize_(Arena, (Count)*sizeof(type))

inline void*
PushSize_(memory_arena* Arena, memory_index Size) {

	Assert((Arena->Used + Size) <= Arena->Size);
	void* Result = Arena->Base + Arena->Used;
	Arena->Used += Size;

	return(Result);

}

inline temporary_memory
BeginTemporaryMemory(memory_arena* Arena)
{
	temporary_memory Result;
	Result.Arena = Arena;
	Result.Used = Arena->Used;

	++Arena->TempCount;

	return(Result);

}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
	memory_arena* Arena = TempMem.Arena;
	Assert(Arena->Used >= TempMem.Used)
		Arena->Used = TempMem.Used;
	Assert(Arena->TempCount > 0);
	--Arena->TempCount;
}

inline void 
CheckArena(memory_arena* Arena)
{
	Assert(Arena->TempCount == 0);
}


#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))

inline void
ZeroSize(memory_index Size, void* Ptr)
{
	uint8* Byte = (uint8*)Ptr;
	while (Size--)
	{
		*Byte++ = 0;
	}
}

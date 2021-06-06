/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade.h"

internal
void GameOutputSound(game_state* GameState, game_sound_output_buffer * SoundBuffer, int ToneHz)
{
	int16_t ToneVolume = 3000;
	int WavePeriod= SoundBuffer->SamplesPerSecond/ToneHz;
	
	int16_t * SampleOut = SoundBuffer->Samples ;
	for(int SampleIndex = 0 ; 
		SampleIndex < SoundBuffer->SampleCount ;
		 ++SampleIndex ){
#if 0
				real32  SineValue = sinf(GameState->tSine);
				int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
#else
				int16_t SampleValue = 0;
#endif
				*SampleOut++= SampleValue;
				*SampleOut++= SampleValue;
#if 0
				GameState-> tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
				//very critical for proper sound output
				if (GameState->tSine > 2.0f * Pi32) {
					GameState->tSine -= 2.0f * Pi32;
				}
#endif	
			}


}

//===============================================================
//!
//! note : Method to Round a floating point number to integer
//! 
//! args : float value
//!
internal int32
RoundReal32ToInt32(real32 Real32) {

	/*
		0.75f + 0.5f = 1.25f => on truncating becomes 1
		0.25f + 0.5f = 0.75f => on truncating becomes 0
		This operation mimics Rounding using tuncation
	*/
	int32 Result = (int32)(Real32 + RoundingFraction);
	return (Result);

}


//===============================================================
//!
//! note : Method to Round a floating point number to unsigned integer 
//! 
//! args : float value
//!
internal uint32
RoundReal32ToUint32(real32 Real32) {

	/*
		0.75f + 0.5f = 1.25f => on truncating becomes 1
		0.25f + 0.5f = 0.75f => on truncating becomes 0
		This operation mimics Rounding using tuncation
	*/
	uint32 Result = (uint32)(Real32 + RoundingFraction);
	return (Result);

}


//===============================================================
//!
//! note : Method to draw a rectangle into game window 
//! 
//! args : Game buffer,  x and y co-ordinates min & max values, RGB color of rect
//!
internal void 
DrawRectangle(game_offscreen_buffer* Buffer,
	real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY , real32 R, real32 G, real32 B) {

	//Rounding NOT truncating
	/*
		0.75 : Rounded value : 1, Truckated Value : 0
	*/
	int32 MinX = RoundReal32ToInt32(RealMinX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MaxY = RoundReal32ToInt32(RealMaxY);


	//Clipping the rectangle
	if (MinX < 0) {
		MinX = 0;
	}
	if (MinY < 0) {
		MinY = 0;
	}
	if (MaxX > Buffer->Width) {
		MaxX = Buffer->Width;
	}
	if (MaxY > Buffer->Height) {
		MaxY = Buffer->Height;
	}


	uint32 Color = (uint32)((RoundReal32ToUint32(R * 255.0f) << 16)|
							(RoundReal32ToUint32(G * 255.0f) << 8)|
							(RoundReal32ToUint32(B * 255.0f) << 0));

	//Taking pointer to the top corner of rectangle location
	uint8* Row = ((uint8*)Buffer->Memory + (MinX * Buffer->BytesPerPixel) + (MinY * Buffer->Pitch));

	//Drawing the Pixels

	for (int Y = MinY; Y < MaxY; ++Y) {

		uint32* Pixel = (uint32*)Row;
		
		for (int X = MinX; X < MaxX; ++X) {
		
			*Pixel++ = Color;
		}

		Row += Buffer->Pitch;
	}
}

extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
	game_state * GameState = (game_state * )Memory->PermanentStorage;
	 
	if(!Memory->IsInitialized){

		Memory->IsInitialized = true;
		/*char * Filename = __FILE__;
		debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Thread, Filename);
		if(File.Contents){
		
			Memory->DEBUGPlatformWriteEntireFile(Thread,"C:/Dev/Tesing.txt",File.ContentSize, File.Contents);
			Memory->DEBUGPlatformFreeFileMemory(Thread,File.Contents);
		
		}
		GameState->ToneHz = 256;
		GameState->tSine = 0.0f;
		Memory->IsInitialized = true;*/
	}
	/*local_persist int BlueOffset = 0 ;
	local_persist int GreenOffset = 0 ;
	local_persist int ToneHz = 256;*/
 
 	for(int ControllerIndex = 0 ;
 		ControllerIndex < ArrayCount(Input->Controllers);
 			++ControllerIndex)
	{
		game_controller_input * Controller = GetController(Input,ControllerIndex);

		if(Controller->IsAnalog){
		
		/*BlueOffset += (int)(4.0f*(Controller->StickAverageX));
		ToneHz = 256 + (int)(128.0f*(Controller->StickAverageY));*/
		
		}else{
		
		/*if(Controller->MoveLeft.EndedDown){
			BlueOffset -=3;
		}
		if(Controller->MoveRight.EndedDown){
			BlueOffset +=3;
		}*/
			
			real32 dPlayerX = 0.0f;
			real32 dPlayerY = 0.0f;

			if (Controller->MoveUp.EndedDown) {
				dPlayerY = -1.0f;
			}
			if (Controller->MoveDown.EndedDown) {
				dPlayerY = 1.0f;
			}
			if (Controller->MoveLeft.EndedDown) {
				dPlayerX = -1.0f;
			}
			if (Controller->MoveRight.EndedDown) {
				dPlayerX = 1.0f;
			}
			//For making speed of player independent of frame rate
			dPlayerX *= 20.0f;
			dPlayerY *= 20.0f;
			
			GameState->PlayerX +=  Input->dtForFrame*dPlayerX;
			GameState->PlayerY += Input->dtForFrame*dPlayerY;

		}

		/*if(Controller->ActionDown.EndedDown){

			GreenOffset += 3;
		}*/
	}
	//TODO : Change later for more complex time based way, sample offset
	
	//For Debug purposes to display Gradient
	//RenderGrdaient(Buffer,BlueOffset, GreenOffset);
	//*****************************************************************TILE DRAWING
	//Test TileMap
	uint32 TileMap[9][17] = {
	
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 0},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
	
	};

	real32 UpperLeftX = -30;
	real32 UpperLeftY = 0;
	real32 TileWidth = 60;
	real32 TileHeight = 60;

	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 0.1f);

	for (int Row = 0; Row < 9; ++Row) {

		for (int Column = 0; Column < 17; ++Column) {

			uint32 TileID = TileMap[Row][Column];
			real32 Grey = 0.5f;
			if (TileID == 1) {
				Grey = 1.0f; 
  			}

			real32 MinX = UpperLeftX + ((real32)Column) * TileWidth;
			real32 MinY = UpperLeftY + ((real32)Row) * TileHeight;
			real32 MaxX = MinX + TileWidth;
			real32 MaxY = MinY + TileHeight;
			DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Grey, Grey, Grey);
		}
		
	}
//*****************************************************************************

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerWidth = 0.75f * TileWidth;
	real32 PlayerHeight = TileHeight/2;
	real32 PlayerLeft = GameState->PlayerX - (0.5f * PlayerWidth);
	real32 PlayerTop = GameState->PlayerY - PlayerHeight;

	//Drawing Test PLayer
	DrawRectangle(Buffer, 
				  PlayerLeft, PlayerTop,
				  PlayerLeft + PlayerWidth,
				  PlayerTop + PlayerHeight, 
				  PlayerR,PlayerG,PlayerB);
	//DrawRectangle(Buffer, 10.0f, 10.0f, 300.0f, 300.0f, 0.0f, 1.0f, 0.0f);

}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state* GameState = (game_state*)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

internal
void RenderGrdaient(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset)
{

	uint8* Row = (uint8*)Buffer->Memory;

	for (int y = 0; y < Buffer->Height; ++y) {

		uint32* Pixel = (uint32*)Row; //LITTLE ENDIAN - Reason for blue bb gg rr
		for (int x = 0; x < Buffer->Width; ++x) {

			uint8 Blue = (uint8)(x + BlueOffset);
			uint8 Green = (uint8)(y + GreenOffset);
			//uint8 Red = (uint8)(x + BlueOffset); //Gives a Pink tint

			*Pixel++ = ((Green << 16) | (Blue));
		}
		Row += Buffer->Pitch;
	}

}
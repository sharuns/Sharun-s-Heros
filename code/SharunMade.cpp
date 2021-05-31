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

				real32  SineValue = sinf(GameState->tSine);
				int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
				*SampleOut++= SampleValue;
				*SampleOut++= SampleValue;

				GameState-> tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
				//very critical for proper sound output
				if (GameState->tSine > 2.0f * Pi32) {
					GameState->tSine -= 2.0f * Pi32;
				}
		
			}


}

internal 
void RenderGrdaient(game_offscreen_buffer * Buffer,int BlueOffset,int GreenOffset)
{

	uint8 * Row = (uint8 *)Buffer->Memory;

	for(int y = 0 ; y < Buffer->Height ; ++y){

		uint32 *Pixel = (uint32 *) Row; //LITTLE ENDIAN - Reason for blue bb gg rr
		for(int x = 0 ; x < Buffer->Width; ++x){

				uint8 Blue = (uint8)(x + BlueOffset);
				uint8 Green = (uint8)(y + GreenOffset);
				//uint8 Red = (uint8)(x + BlueOffset); //Gives a Pink tint

				*Pixel++ = ((Green << 16) | (Blue));
		}
		Row += Buffer->Pitch;
	}

}


extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;
	if(!Memory->IsInitialized){
		char * Filename = __FILE__;
		debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Filename);
		if(File.Contents){
		
			Memory->DEBUGPlatformWriteEntireFile("C:/Dev/Tesing.txt",File.ContentSize, File.Contents);
			Memory->DEBUGPlatformFreeFileMemory(File.Contents);
		
		}
		GameState->ToneHz = 256;
		GameState->tSine = 0.0f;
		Memory->IsInitialized = true;
	}
	local_persist int BlueOffset = 0 ;
	local_persist int GreenOffset = 0 ;
	local_persist int ToneHz = 256;
 
 	for(int ControllerIndex = 0 ;
 		ControllerIndex < ArrayCount(Input->Controllers);
 			++ControllerIndex){
	game_controller_input * Controller = GetController(Input,ControllerIndex);

	if(Controller->IsAnalog){
		
		BlueOffset += (int)(4.0f*(Controller->StickAverageX));
		ToneHz = 256 + (int)(128.0f*(Controller->StickAverageY));
		
	}else{
		
		if(Controller->MoveLeft.EndedDown){
			BlueOffset -=3;
		}
		if(Controller->MoveRight.EndedDown){
			BlueOffset +=3;
		}

	}

	if(Controller->ActionDown.EndedDown){

		GreenOffset += 3;
		}
	}
	//TODO : Change later for more complex time based way, sample offset
	
	RenderGrdaient(Buffer,BlueOffset, GreenOffset);

}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state* GameState = (game_state*)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, GameState->ToneHz);
}
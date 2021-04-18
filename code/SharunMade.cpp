/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade.h"

internal
void GameOutputSound(game_sound_output_buffer * SoundBuffer, int ToneHz)
{
	local_persist real32 tSine;
	int16_t ToneVolume = 3000;
	int WavePeriod= SoundBuffer->SamplesPerSecond/ToneHz;
	
	int16_t * SampleOut = SoundBuffer->Samples ;
	for(int SampleIndex = 0 ; 
		SampleIndex < SoundBuffer->SampleCount ;
		 ++SampleIndex ){
				real32  SineValue = sinf(tSine);
				int16_t SampleValue = (int16_t)(SineValue * ToneVolume);
				*SampleOut++= SampleValue;
				*SampleOut++= SampleValue;

				tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
				//very critical for proper sound output
		
			}


}

internal 
void RenderGrdaient(game_offscreen_buffer * Buffer,int BlueOffset,int GreenOffset,
					game_sound_output_buffer * SoundBuffer)
{

	uint8 * Row = (uint8 *)Buffer->Memory;

	for(int y = 0 ; y < Buffer->Height ; ++y){

		uint32 *Pixel = (uint32 *) Row; //LITTLE ENDIAN - Reason for blue bb gg rr
		for(int x = 0 ; x < Buffer->Width; ++x){

				uint8 Blue = (uint8)(x + BlueOffset);
				uint8 Green = (uint8)(y + GreenOffset);
				uint8 Red = (uint8)(x + BlueOffset); //Gives a Pink tint

				*Pixel++ = ((Green << 8) | Blue | (Red << 16));
		}
		Row += Buffer->Pitch;
	}

}

internal
void GameUpdateAndRenderer(game_memory * Memory,
						   game_input * Input,
						   game_offscreen_buffer * Buffer,
						   game_sound_output_buffer * SoundBuffer){

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;
	if(!Memory->IsInitialized){
		char * Filename = __FILE__;
		debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
		if(File.Contents){
		
			DEBUGPlatformWriteEntireFile("C:/Dev/Tesing.txt",File.ContentSize, File.Contents);
			DEBUGPlatformFreeFileMemory(File.Contents);
		
		}
		GameState->ToneHz = 256;
		Memory->IsInitialized = true;
	}
	local_persist int BlueOffset = 0 ;
	local_persist int GreenOffset = 0 ;
	local_persist int ToneHz = 256;
 
	game_controller_input * Input0 = &Input->Controllers[0];

	if(Input0->IsAnalog){
		BlueOffset += (int)(4.0f*(Input0->EndX));
		ToneHz = 256 + (int)(128.0f*(Input0->EndY));
		
	}else{



	}

	if(Input0->Down.EndedDown){

		GreenOffset += 3;
	}

	//TODO : Change later for more complex time based way, sample offset
	GameOutputSound(SoundBuffer,ToneHz);
	RenderGrdaient(Buffer,BlueOffset, GreenOffset, SoundBuffer);

}
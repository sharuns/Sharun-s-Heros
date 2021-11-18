/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade.h"
#include "SharunMade_Intrinsics.h"

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

internal void 
DrawRectangle(game_offscreen_buffer* Buffer,
	real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY ,
	real32 R, real32 G, real32 B) {

	//Rounding NOT truncating
	/*
		0.75 : Rounded value : 1, Truckated Value : 0
	*/
	int32 MinX = RoundReal32ToInt32(RealMinX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MaxY = RoundReal32ToInt32(RealMaxY);


	//Clipping the rectangle so that we are drawing in the valid buffer section
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


	uint32 Color = (	(RoundReal32ToUint32(R * 255.0f) << 16)|
						(RoundReal32ToUint32(G * 255.0f) << 8)|
						(RoundReal32ToUint32(B * 255.0f)));

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


/*

	Returns the tile in the tileMap whose corordinates have been passed

*/
inline uint32
GetTileValueUnchecked(world * World,tile_map * TileMap, int32 TileX, int32 TileY){

	Assert(TileMap);
	Assert((TileX >= 0 ) && (TileX < World->CountX) && 
	       (TileY >= 0 ) && (TileY < World->CountY));
	uint32 TileMapValue = TileMap->Tiles[TileY * World->CountX + TileX];
	return (TileMapValue);

}

/*

	Returns the tileMap in the World tileMap space whose corordinates have been passed

*/
inline tile_map*
GetTileMap(world * World,int32 TileMapX, int32 TileMapY){

	tile_map *  TileMap = 0;

	if((TileMapX >= 0 ) && (TileMapX < World->TileMapCountX) && 
	   (TileMapY >= 0 ) && (TileMapY < World->TileMapCountY))
	{
		TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
		
	}
	return (TileMap);
}


/*
	This function checks on the basis of X and Y if we are presnt inside the tile map or not
	so as to find the boundaries of that tile map

*/
inline bool32 
IsTileMapPointEmpty(world * World, tile_map * TileMap,real32 TestX, real32 TestY){

	bool32 Empty = false;
	if(TileMap){

		if((TestX >= 0) && (TestX < World->CountX) &&
		   (TestY >= 0) && (TestY < World->CountY)){
	
			uint32 TileMapValue = GetTileValueUnchecked(World,TileMap,TestX,TestY);
			Empty = (TileMapValue == 0);
		}
	}
	return (Empty);
}


inline void
ReCannonicalizeCoord(world * World, int32 TileCount, int32 * TileMap, int32 * Tile,real32 * TileRel){

	int32 Offset = FloorReal32ToInt32(*TileRel / World->TileSideInMeters);	
	*Tile += Offset; 
	*TileRel -= Offset*World->TileSideInMeters;


	//Need to check this Assert hit case
	Assert(*TileRel >= 0);	
	Assert(*TileRel <= World->TileSideInPixels);


	if(*Tile < 0){

		*Tile = TileCount + *Tile;
		--*TileMap;
	}
	
	if(*Tile >= TileCount){

		*Tile = *Tile - TileCount;
		++*TileMap;
	}

	
}


inline cannonical_position 
ReCannonicalizePosition(world * World,cannonical_position Pos){
	
	cannonical_position Result = Pos;

	ReCannonicalizeCoord(World,World->CountX,&Result.TileMapX,&Result.TileX, &Result.TileRelX);
	ReCannonicalizeCoord(World,World->CountY,&Result.TileMapY,&Result.TileY, &Result.TileRelY);

	return (Result);
}


/*
	This is one top of the TileMap space and is used to identify that currently we are on which tile map based and on the
	X and Y, And to which we will end up if we make the move

*/
inline bool32 
IsWorldPointEmpty(world * World, cannonical_position CanPos){

	bool32 Empty = false;

	tile_map * TileMap = GetTileMap(World,CanPos.TileMapX,CanPos.TileMapY);

	Empty = IsTileMapPointEmpty(World, TileMap,CanPos.TileX,CanPos.TileY);
	return (Empty);
}



extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	
#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9



 	uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {

	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
	{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
	{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 0},
	{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
	{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
	{1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},

	};
	uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {

	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},

	};
	uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {

	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},

	};
	uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {

	{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
	{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},

	};

	tile_map TileMaps[2][2];

	TileMaps[0][0].Tiles = (uint32 *)Tiles00;
	TileMaps[0][1].Tiles = (uint32 *)Tiles10;
	TileMaps[1][0].Tiles = (uint32 *)Tiles01;
	TileMaps[1][1].Tiles = (uint32 *)Tiles11;

	world World;
	World.TileMapCountX = 2;
	World.TileMapCountY = 2;
	World.CountX = TILE_MAP_COUNT_X;
	World.CountY = TILE_MAP_COUNT_Y;
	World.TileSideInMeters = 1.4f;// this like a calibration value for our world
	World.TileSideInPixels = 60;
	World.MetersToPixels =(real32)World.TileSideInPixels/(real32)World.TileSideInMeters;  //Pixels per meter
	World.UpperLeftX = -(real32)World.TileSideInPixels/2;
	World.UpperLeftY = 0;

	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f * PlayerHeight;

	World.TileMaps = (tile_map * )TileMaps; // Stores all the tile maps




	game_state * GameState = (game_state * )Memory->PermanentStorage;

	if(!Memory->IsInitialized){

		GameState->PlayerP.TileMapX = 0;
		GameState->PlayerP.TileMapY = 0;
		GameState->PlayerP.TileX = 3;
		GameState->PlayerP.TileY = 3;
		GameState->PlayerP.TileRelX = 5.0f;
		GameState->PlayerP.TileRelY = 5.0f;
		
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
	tile_map * TileMap = GetTileMap(&World,GameState->PlayerP.TileMapX,GameState->PlayerP.TileMapY);
	Assert(TileMap);




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
				//digital controller = Keys

			real32 dPlayerX = 0.0f;
			real32 dPlayerY = 0.0f;


			if(Controller->MoveUp.EndedDown){

				
				dPlayerY = -1.0f;
			}
			if(Controller->MoveDown.EndedDown){
			
				dPlayerY = 1.0f;

			}
			if(Controller->MoveLeft.EndedDown){
				dPlayerX = -1.0f;
			}
			if(Controller->MoveRight.EndedDown){
				dPlayerX = 1.0f;
			}
			//dPlayerX *= 2.0f;
			//dPlayerY *= 2.0f; //10 m/s speed for motion 

			cannonical_position NewPlayerP = GameState->PlayerP;
			NewPlayerP.TileRelX += Input->dtForFrame*dPlayerX;
			NewPlayerP.TileRelY += Input->dtForFrame*dPlayerY;
			NewPlayerP = ReCannonicalizePosition(&World,NewPlayerP);

			cannonical_position PlayerLeft = NewPlayerP;
			PlayerLeft.TileRelX -= 0.5f*PlayerWidth;
			PlayerLeft = ReCannonicalizePosition(&World,PlayerLeft);

			cannonical_position PlayerRight = NewPlayerP;
			PlayerRight.TileRelX += 0.5f*PlayerWidth;
			PlayerRight = ReCannonicalizePosition(&World,PlayerRight);

			/*Detecting boundary of a tileMap

				_________
				|        |
				|        |
				|   P    |
				|        |
				x---xx---x
				1    2   3

				We are only checking the 3 points in the bottom (marked with x) for the 
				3 D sort of prespective appearence for the character wrt 
				to obstacles

			*/
			if(IsWorldPointEmpty(&World,NewPlayerP)&&
			   IsWorldPointEmpty(&World,PlayerLeft) && 
			   IsWorldPointEmpty(&World,PlayerRight))
			{

				GameState->PlayerP = NewPlayerP;
			}
		}

		/*if(Controller->ActionDown.EndedDown){

			GreenOffset += 3;
		}*/
	}
	//TODO : Change later for more complex time based way, sample offset
	
	//For Debug purposes to display Gradient
	//RenderGrdaient(Buffer,BlueOffset, GreenOffset);

	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width,(real32)Buffer->Height, 1.0f,0.0f,1.0f);


	for(int Row = 0 ; Row < 9 ; ++Row){

		for(int Column = 0 ; Column < 17 ; ++Column){

			uint32 TileID = GetTileValueUnchecked(&World, TileMap, Column, Row);
			real32 Gray = 0.5f;
			if(TileID == 1){

				Gray = 1.0f;
			}

			if((Column == GameState->PlayerP.TileX)&&
				(Row == GameState->PlayerP.TileY)){
				
				Gray = 0.0f;
			}
			real32 MinX = World.UpperLeftX + ((real32)Column)*World.TileSideInPixels;
			real32 MinY = World.UpperLeftY + ((real32)Row)*World.TileSideInPixels;
			real32 MaxX = MinX + World.TileSideInPixels;
			real32 MaxY = MinY + World.TileSideInPixels;


			DrawRectangle(Buffer, MinX,MinY,MaxX,MaxY,Gray,Gray,Gray);
			
		}

	}

	//GameState->PlayerX = 10.0f;
	//GameState->PlayerY = 10.0f;

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;

	real32 PlayerLeft = World.UpperLeftX + World.TileSideInPixels*GameState->PlayerP.TileX + 
						World.MetersToPixels*GameState->PlayerP.TileRelX - (0.5f *World.MetersToPixels*PlayerWidth);
	real32 PlayerTop = World.UpperLeftY +  World.TileSideInPixels*GameState->PlayerP.TileY + 
						World.MetersToPixels*GameState->PlayerP.TileRelY -  (World.MetersToPixels*PlayerHeight);

	DrawRectangle(Buffer,
				  PlayerLeft, PlayerTop,
				  PlayerLeft + World.MetersToPixels*PlayerWidth,
				  PlayerTop + World.MetersToPixels*PlayerHeight,PlayerR,PlayerG,PlayerB);

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
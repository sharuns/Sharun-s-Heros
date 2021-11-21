/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade.h"
#include "SharunMadeTile.cpp"

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

inline bool32 
IsWorldPointEmpty(tile_map * TileMap, tile_map_position CanPos){

	uint32 TileChunkValue = GetTileValue(TileMap,CanPos.AbsTileX,CanPos.AbsTileY);
	bool32 Empty = (TileChunkValue ==0);
	return (Empty);
}

internal void
InitailizeArena(memory_arena * Arena,memory_index Size,uint8 * Base){

	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0 ;
}

//Primitive memory allocator for now, which pushed our tiles from previously stored in stack to the 
//dedicated allocated game memory 

#define PushStruct(Arena,type) (type * )PushSize_(Arena, sizeof(type))
#define PushArray(Arena,Count,type) (type*)PushSize_(Arena, (Count)*sizeof(type))

void*
PushSize_(memory_arena * Arena, memory_index Size){

	Assert((Arena->Used + Size) <= Arena->Size);
	void * Result = Arena->Base + Arena->Used;
	Arena->Used +=Size;

	return(Result);

}



extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;


		real32 PlayerHeight = 1.4f;
		real32 PlayerWidth = 0.75f * PlayerHeight;

	if(!Memory->IsInitialized){


		GameState->PlayerP.AbsTileX = 1;
		GameState->PlayerP.AbsTileY = 3;
		GameState->PlayerP.TileRelX = 5.0f;
		GameState->PlayerP.TileRelY = 5.0f;
		
		InitailizeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
						(uint8 * )Memory->PermanentStorage + sizeof(game_state));


		GameState->World = PushStruct(&GameState-> WorldArena, world);
		world * World = GameState->World;
		World->TileMap = PushStruct(&GameState-> WorldArena,tile_map);

		tile_map * TileMap = World->TileMap;

		TileMap->ChunkShift = 4;
		TileMap->ChunkMask = (1 << TileMap->ChunkShift)-1;
		TileMap->ChunkDim = (1 << TileMap->ChunkShift);

		TileMap->TileChunkCountX = 128;
		TileMap->TileChunkCountY = 128;

		TileMap->TileChunks = PushArray(&GameState->WorldArena,
									   TileMap->TileChunkCountX*TileMap->TileChunkCountY,
									   	tile_chunk);
	

		
		for(uint32 Y = 0 ; 
			Y < TileMap->TileChunkCountY;
			++Y)
		
		{
			for(uint32 X = 0 ; 
			X < TileMap->TileChunkCountX;
			++X)	
			{
				TileMap->TileChunks[Y*TileMap->TileChunkCountX + X].Tiles = 
					PushArray(&GameState->WorldArena,TileMap->ChunkDim*TileMap->ChunkDim,uint32);
			}

		}
		

		TileMap->TileSideInMeters = 1.4f;// this like a calibration value for our world
		TileMap->TileSideInPixels = 60;
		TileMap->MetersToPixels =(real32)TileMap->TileSideInPixels/(real32)TileMap->TileSideInMeters;  //Pixels per meter


	
		real32 LowerLeftX = -(real32)TileMap->TileSideInPixels/2;
		real32 LowerLeftY = (real32)Buffer->Height;

		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;

		for(uint32 ScreenY =0;
			ScreenY < 32;
			++ScreenY){

			for(uint32 ScreenX = 0;
				ScreenX < 32;
				++ScreenX){

				for(uint32 TileY = 0;
					TileY < TilesPerHeight;
					++TileY)

					for(uint32 TileX = 0 ;
						TileX < TilesPerWidth;
						++TileX)
					{

						uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
						uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;

						SetTileValue(&GameState->WorldArena, World->TileMap,AbsTileX, AbsTileY,
							(TileX == TileY) && (TileY %2 )?1:0);
					}
			}
		}

	Memory->IsInitialized = true;

	}

	world * World = GameState->World;
	tile_map * TileMap = World->TileMap;


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

				
				dPlayerY = 1.0f;
			}
			if(Controller->MoveDown.EndedDown){
			
				dPlayerY = -1.0f;

			}
			if(Controller->MoveLeft.EndedDown){
				dPlayerX = -1.0f;
			}
			if(Controller->MoveRight.EndedDown){
				dPlayerX = 1.0f;
			}
			real32 PlayerSpeed = 1.0f;
			if(Controller->ActionUp.EndedDown){
				PlayerSpeed = 3.0f;
			}

			dPlayerX *= PlayerSpeed;
			dPlayerY *= PlayerSpeed; //10 m/s speed for motion 

			tile_map_position NewPlayerP = GameState->PlayerP;
			NewPlayerP.TileRelX += Input->dtForFrame*dPlayerX;
			NewPlayerP.TileRelY += Input->dtForFrame*dPlayerY;
			NewPlayerP = ReCannonicalizePosition(TileMap,NewPlayerP);

			tile_map_position PlayerLeft = NewPlayerP;
			PlayerLeft.TileRelX -= 0.5f*PlayerWidth;
			PlayerLeft = ReCannonicalizePosition(TileMap,PlayerLeft);

			tile_map_position PlayerRight = NewPlayerP;
			PlayerRight.TileRelX += 0.5f*PlayerWidth;
			PlayerRight = ReCannonicalizePosition(TileMap,PlayerRight);

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
			if(IsWorldPointEmpty(TileMap,NewPlayerP)&&
			   IsWorldPointEmpty(TileMap,PlayerLeft) && 
			   IsWorldPointEmpty(TileMap,PlayerRight))
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

	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width,(real32)Buffer->Height, 0.0f,0.0f,0.0f);

	real32 ScreenCenterX = 0.5f *(real32)Buffer->Width ;
	real32 ScreenCenterY = 0.5f* (real32)Buffer->Height;

	for(int32 RelRow = -10 ; RelRow < 10 ; ++RelRow){

		for(int32 RelColumn =  -20 ; RelColumn <  20 ; ++RelColumn){

			uint32 Column = GameState->PlayerP.AbsTileX +RelColumn;
			uint32 Row = GameState->PlayerP.AbsTileY + RelRow;
			uint32 TileID = GetTileValue(TileMap, Column, Row);
			real32 Gray = 0.5f;
			if(TileID == 1){

				Gray = 1.0f;
			}

			if((Column == GameState->PlayerP.AbsTileX)&&
				(Row == GameState->PlayerP.AbsTileY)){
				
				Gray = 0.0f;
			}
		
		//smooth scrolling effect
			real32 CenX = ScreenCenterX - TileMap->MetersToPixels*GameState->PlayerP.TileRelX + ((real32)RelColumn)*TileMap->TileSideInPixels;
			real32 CenY = ScreenCenterY + TileMap->MetersToPixels*GameState->PlayerP.TileRelY - ((real32)RelRow)*TileMap->TileSideInPixels;
			real32 MinX = CenX - 0.5f*TileMap->TileSideInPixels;
			real32 MinY = CenY - 0.5f*TileMap->TileSideInPixels;
			real32 MaxX = CenX + 0.5f*TileMap->TileSideInPixels;
			real32 MaxY = CenY + 0.5f*TileMap->TileSideInPixels;

			//real32 MinX = CenterX + ((real32)RelColumn)*TileMap->TileSideInPixels;
			//real32 MinY = CenterY - ((real32)RelRow)*TileMap->TileSideInPixels;


			DrawRectangle(Buffer,MinX,MinY,MaxX,MaxY,Gray,Gray,Gray);
			
		}

	}

	//GameState->PlayerX = 10.0f;
	//GameState->PlayerY = 10.0f;

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;

	//real32 PlayerLeft = CenterX + TileMap->MetersToPixels*GameState->PlayerP.TileRelX - (0.5f *TileMap->MetersToPixels*PlayerWidth);
	//real32 PlayerTop = CenterY - TileMap->MetersToPixels*GameState->PlayerP.TileRelY -  (TileMap->MetersToPixels*PlayerHeight);
	//smooth scrolling effect
	real32 PlayerLeft = ScreenCenterX - (0.5f *TileMap->MetersToPixels*PlayerWidth);
	real32 PlayerTop = ScreenCenterY  -  (TileMap->MetersToPixels*PlayerHeight);

	DrawRectangle(Buffer,
				  PlayerLeft, PlayerTop,
				  PlayerLeft + TileMap->MetersToPixels*PlayerWidth,
				  PlayerTop + TileMap->MetersToPixels*PlayerHeight,PlayerR,PlayerG,PlayerB);

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
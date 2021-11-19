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
//===============================================>Checked
inline uint32
GetTileValueUnchecked(world * World,tile_chunk * TileChunk, uint32 TileX, uint32 TileY){

	Assert(TileChunk);
	Assert((TileX < World->ChunkDim));
	Assert(TileY < World->ChunkDim);
	uint32 TileChunkValue = TileChunk->Tiles[TileY * World->ChunkDim + TileX];
	return (TileChunkValue);

}

/*

	Returns the tileMap in the World tileMap space whose corordinates have been passed

*/
//===============================================>Checked
inline tile_chunk*
GetTileChunk(world * World,int32 TileChunkX, int32 TileChunkY){

	tile_chunk *  TileChunk = 0;

	if((TileChunkX >= 0 ) && (TileChunkX < World->TileChunkCountX) && 
	   (TileChunkY >= 0 ) && (TileChunkY < World->TileChunkCountY))
	{
		TileChunk = &World->TileChunks[TileChunkY * World->TileChunkCountX + TileChunkX];
		
	}
	return (TileChunk);
}

//===============================================>Checked
inline uint32
GetTileValue(world * World,tile_chunk * TileChunk,uint32 TestTileX, uint32 TestTileY){

	uint32 TileChunkValue = 0 ;

	if(TileChunk){
		TileChunkValue = GetTileValueUnchecked(World,TileChunk,TestTileX,TestTileY);
	}
	return(TileChunkValue);
}

//===============================================>Checked
inline tile_chunk_position
GetChunkPositionFor(world * World,uint32 AbsTileX,uint32 AbsTileY){

	tile_chunk_position Result;

	Result.TileChunkX  = AbsTileX >> World->ChunkShift; 
	Result.TileChunkY =  AbsTileY >> World->ChunkShift;
	Result.RelTileX =  AbsTileX & World->ChunkMask;
	Result.RelTileY =  AbsTileY & World->ChunkMask;

	return (Result);
}



//===============================================>Checked
internal uint32
GetTileValue(world * World,uint32 AbsTileX, uint32 AbsTileY){

	bool32 Empty = false;
	tile_chunk_position ChunkPos = GetChunkPositionFor(World,AbsTileX,AbsTileY);
	tile_chunk * TileMap = GetTileChunk(World,ChunkPos.TileChunkX,ChunkPos.TileChunkY);
	uint32 TileChunkValue = GetTileValue(World,TileMap,ChunkPos.RelTileX,ChunkPos.RelTileY);

	return(TileChunkValue);
}



/*
	This function checks on the basis of X and Y if we are presnt inside the tile map or not
	so as to find the boundaries of that tile map

*/
inline bool32 
IsTileChunkTileEmpty(world * World, tile_chunk * TileChunk,uint32 TestX, uint32 TestY){

	bool32 Empty = false;
	if(TileChunk){
	
		uint32 TileChunkValue = GetTileValueUnchecked(World,TileChunk,TestX,TestY);
		Empty = (TileChunkValue == 0);
	}
	return (Empty);
}


inline void
ReCannonicalizeCoord(world * World, uint32 * Tile,real32 * TileRel){

	int32 Offset = FloorReal32ToInt32(*TileRel / World->TileSideInMeters);	

	*Tile += Offset; 
	*TileRel -= Offset*World->TileSideInMeters;


	//Need to check this Assert hit case
	Assert(*TileRel >= 0);	
	Assert(*TileRel <= World->TileSideInPixels);
	
}

//===============================================>Checked
inline world_position 
ReCannonicalizePosition(world * World,world_position Pos){
	
	world_position Result = Pos;

	ReCannonicalizeCoord(World,&Result.AbsTileX, &Result.TileRelX);
	ReCannonicalizeCoord(World,&Result.AbsTileY, &Result.TileRelY);

	return (Result);
}




/*
	This is one top of the TileMap space and is used to identify that currently we are on which tile map based and on the
	X and Y, And to which we will end up if we make the move


inline bool32 
IsWorldPointEmpty(world * World, world_position CanPos){

	bool32 Empty = false;

	//query for the chunk
	tile_chunk_position ChunkPos = GetChunkPositionFor(World,ChunkPos.AbsTileX, ChunkPos.AbsTileY);
	
	tile_chunk * TileChunk = GetTileChunk(World,ChunkPos.RelTileX, ChunkPos.RelTileY);
	//query inside the chunk
	Empty = IsTileChunkTileEmpty(World, TileChunk,ChunkPos.RelTileX,ChunkPos.RelTileY);
	return (Empty);
}
*/

	/*tile_map * TileMap = GetTileMap(&World,GameState->PlayerP.TileMapX,GameState->PlayerP.TileMapY);
	Assert(TileMap);*/




inline bool32 
IsWorldPointEmpty(world * World, world_position CanPos){

	uint32 TileChunkValue = GetTileValue(World,CanPos.AbsTileX,CanPos.AbsTileY);
	bool32 Empty = (TileChunkValue ==0);
	return (Empty);
}




extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	
#define TILE_MAP_COUNT_X 256
#define TILE_MAP_COUNT_Y 256


	//club these together 
 	uint32 TempTiles[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {

		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,	1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1,	1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 0,	0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
		{1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1,	1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
		{1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1,	1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
		{1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,	1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,	1, 1, 1, 1,  0, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,	1, 1, 1, 1,  0, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,	0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,	1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
	};

	world World;
	World.ChunkShift = 0x08;
	World.ChunkMask = (1 << World.ChunkShift);
	World.ChunkMask = World.ChunkMask -1 ;
	World.ChunkDim = 256;

	World.TileChunkCountX = 1;
	World.TileChunkCountY = 1;
/*
struct tile_chunk{
	uint32 * Tiles;
};

struct world{


	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDim;

	real32 TileSideInMeters; 
	int32 TileSideInPixels;
	real32 MetersToPixels;

	int32 TileChunkCountX;
	int32 TileChunkCountY;


	tile_chunk * TileChunks;

};
*/
	tile_chunk TileChunk;
	TileChunk.Tiles = (uint32 *)TempTiles;
	World.TileChunks = &TileChunk;
	

	World.TileSideInMeters = 1.4f;// this like a calibration value for our world
	World.TileSideInPixels = 60;
	World.MetersToPixels =(real32)World.TileSideInPixels/(real32)World.TileSideInMeters;  //Pixels per meter


	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f * PlayerHeight;

	real32 LowerLeftX = -(real32)World.TileSideInPixels/2;
	real32 LowerLeftY = (real32)Buffer->Height;

	game_state * GameState = (game_state * )Memory->PermanentStorage;

	if(!Memory->IsInitialized){


		GameState->PlayerP.AbsTileX = 3;
		GameState->PlayerP.AbsTileY = 3;
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
			//dPlayerX *= 2.0f;
			//dPlayerY *= 2.0f; //10 m/s speed for motion 

			world_position NewPlayerP = GameState->PlayerP;
			NewPlayerP.TileRelX += Input->dtForFrame*dPlayerX;
			NewPlayerP.TileRelY += Input->dtForFrame*dPlayerY;
			NewPlayerP = ReCannonicalizePosition(&World,NewPlayerP);

			world_position PlayerLeft = NewPlayerP;
			PlayerLeft.TileRelX -= 0.5f*PlayerWidth;
			PlayerLeft = ReCannonicalizePosition(&World,PlayerLeft);

			world_position PlayerRight = NewPlayerP;
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

	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width,(real32)Buffer->Height, 0.0f,0.0f,0.0f);

	real32 CenterX = 0.5f *(real32)Buffer->Width ;
	real32 CenterY = 0.5f* (real32)Buffer->Height;

	for(int32 RelRow = -10 ; RelRow < 10 ; ++RelRow){

		for(int32 RelColumn =  -20 ; RelColumn <  20 ; ++RelColumn){

			uint32 Column = GameState->PlayerP.AbsTileX +RelColumn;
			uint32 Row = GameState->PlayerP.AbsTileY + RelRow;
			uint32 TileID = GetTileValue(&World, Column, Row);
			real32 Gray = 0.5f;
			if(TileID == 1){

				Gray = 1.0f;
			}

			if((Column == GameState->PlayerP.AbsTileX)&&
				(Row == GameState->PlayerP.AbsTileY)){
				
				Gray = 0.0f;
			}
		
			real32 MinX = CenterX + ((real32)RelColumn)*World.TileSideInPixels;
			real32 MinY = CenterY - ((real32)RelRow)*World.TileSideInPixels;
			real32 MaxX = MinX + World.TileSideInPixels;
			real32 MaxY = MinY - World.TileSideInPixels;


			DrawRectangle(Buffer,MinX,MaxY,MaxX,MinY,Gray,Gray,Gray);
			
		}

	}

	//GameState->PlayerX = 10.0f;
	//GameState->PlayerY = 10.0f;

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;

	real32 PlayerLeft = CenterX + World.MetersToPixels*GameState->PlayerP.TileRelX - (0.5f *World.MetersToPixels*PlayerWidth);
	real32 PlayerTop = CenterY - World.MetersToPixels*GameState->PlayerP.TileRelY -  (World.MetersToPixels*PlayerHeight);

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
/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade.h"
#include "SharunMadeTile.cpp"
#include "RandomNo.h"
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
	V2 vMin, V2 vMax,
	real32 R, real32 G, real32 B) {

	//Rounding NOT truncating
	/*
		0.75 : Rounded value : 1, Truckated Value : 0
	*/
	int32 MinX = RoundReal32ToInt32(vMin.X);
	int32 MinY = RoundReal32ToInt32(vMin.Y);
	int32 MaxX = RoundReal32ToInt32(vMax.X);
	int32 MaxY = RoundReal32ToInt32(vMax.Y);


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


internal void
DrawBitmap(game_offscreen_buffer * Buffer, loaded_bitmap * Bitmap, real32 RealX, real32 RealY,
			int32 AlignX = 0, int32 AlignY = 0){

	RealX -= (real32)AlignX;
	RealY -= (real32)AlignY;

	int32 MinX = RoundReal32ToInt32(RealX);
	int32 MinY = RoundReal32ToInt32(RealY);
	int32 MaxX = RoundReal32ToInt32(RealX + (real32)Bitmap->Width);
	int32 MaxY = RoundReal32ToInt32(RealY + (real32)Bitmap->Height);

	//Clipping the rectangle so that we are drawing in the valid buffer section
	int32 SourceOffsetX = 0;
	if (MinX < 0) {
		SourceOffsetX = -MinX;
		MinX = 0;
	}
	int32 SourceOffsetY = 0;
	if (MinY < 0) {
		SourceOffsetY = -MinY;
		MinY = 0;
	}
	if (MaxX > Buffer->Width) {
		MaxX = Buffer->Width;
	}
	if (MaxY > Buffer->Height) {
		MaxY = Buffer->Height;
	}

	uint32 * SourceRow  = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height -1);
	SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;
	uint8 * DestRow = ((uint8 *)Buffer->Memory + 
						MinX*Buffer->BytesPerPixel + 
						MinY*Buffer->Pitch);
	for(int32 Y = MinY ;
		Y < MaxY;
		++Y){

		uint32 *Dest = (uint32 *)DestRow;
		uint32 * Source = SourceRow;
		for(int32 X = MinX ; 
			X < MaxX;
			++X){

			//################linear alpha blending################

			real32 A = (real32)((*Source >> 24) & 0xFF)/255.0f;
			real32 SR = (real32)((*Source >> 16) & 0xFF);
			real32 SG = (real32)((*Source >> 8) & 0xFF);
			real32 SB = (real32)((*Source >> 0) & 0xFF);

			real32 DR = (real32)((*Dest >> 16) & 0xFF);
			real32 DG = (real32)((*Dest >> 8) & 0xFF);
			real32 DB = (real32)((*Dest >> 0) & 0xFF);

			real32 R = (1.0f - A)*DR + A*SR;
			real32 G = (1.0f - A)*DG + A*SG;
			real32 B = (1.0f - A)*DB + A*SB;

			*Dest = (((uint32)(R+0.5f) << 16)|
					 ((uint32)(G+0.5f) << 8) |
					 ((uint32)(B+0.5f) << 0));
			
			//####################################################

			*Dest++;
			*Source++;
		}

		DestRow += Buffer->Pitch;
		SourceRow -= Bitmap->Width;
	}

}

//pragma pack push tells the compiler to pack the ele in struct as per there size without padding
//and on pop we return to whatever the packing scheme was present earlier
#pragma pack(push, 1)
struct bitmap_header{

	uint16 FileType;
	uint32 FileSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 Size;
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
	uint32 Compression;
	uint32 SizeOfBitmap;
	int32 HorzResolution;
	int32 VetResolution;
	uint32 ColorUsed;
	uint32 ColorsImportant;

	uint32 RedMask;
	uint32 GreenMask;
	uint32 BlueMask;


};
#pragma pack(pop)




internal loaded_bitmap
DEBUGLoadBMP(thread_context * Thread, debug_platform_read_entire_file * ReadEntireFile,char *FileName){

	loaded_bitmap Result = {};
	//format : AA RR GG BB
	//IMage is getting flipped vertically
	debug_read_file_result ReadResult = ReadEntireFile(Thread,FileName);
	if(ReadResult.ContentSize != 0 ){

		bitmap_header * Header = (bitmap_header*)ReadResult.Contents;
		uint32 * Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
		Result.Pixels = Pixels;
		Result.Width = Header->Width;
		Result.Height = Header->Height;
	
#if 0
		//Sometimes we might have to adjust the byte order ourselves
		int32 ResShift = ;
		int32 GreenShift = ;
		int32 BlueShift = ;
		int32 AlphaShift = ;
		

		uint32 * SourceDest= Pixels;
		for(int32 Y  = 0 ;
			Y < Header->Width;
			++Y){

			for(int32 X = 0 ;
				X < Header->Height;
				++X){

				*SourceDest = (*SourceDest >> 8 )|(*SourceDest << 24);
				++SourceDest;
			}
		}
#endif
	}
	return (Result);
}



internal void
InitailizeArena(memory_arena * Arena,memory_index Size,uint8 * Base){

	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0 ;
}


inline entity*
GetEntity(game_state* GameState, uint32 Index) {


	entity* Entity = 0;
	if ((Index > 0) && (Index < ArrayCount(GameState->Entities))) {
		Entity = &GameState->Entities[Index];
	}
	return (Entity);
}

internal void
InitializePlayer(entity* Entity) {

	/*
	Entity->P.AbsTileX = 1;
	Entity->P.AbsTileY = 3;
	Entity->P.Offset.X = 5.0f;
	Entity->P.Offset.Y = 5.0f;

	*/
	Entity->Exists = true;
	Entity->P.AbsTileX = 1;
	Entity->P.AbsTileY = 3;
	Entity->P.Offset.X = 5.0f;
	Entity->P.Offset.Y = 5.0f;
	Entity->Height = 1.4f;
	Entity->Width = 0.75f * Entity->Height;
}




#if 1 
internal void
MovePlayer(game_state* GameState, entity* Entity, real32 dt, V2 ddP) {

	world* World = GameState->World->World;

	if ((ddP.X != 0.0f) && (ddP.Y != 0.0f)) {

		ddP *= 0.707106781187f;
			}

	
	real32 PlayerSpeed = 50.0f;// m/s^2
	ddP *= PlayerSpeed;

	ddP += -8.0f * Entity->dP;

	world_position OldPlayerP = Entity->P;
	world_position NewPlayerP = OldPlayerP;

	V2 PlayerDelta = (0.5f * ddP * Square(dt) +
					  Entity->dP*dt);
	//position equation p' = (1/2 * at^2 + Vt + p)
	NewPlayerP.Offset += PlayerDelta;

	// velocity equation v' = (at + v)
	Entity->dP = ddP*dt + Entity->dP;

	NewPlayerP = ReCannonicalizePosition(World, NewPlayerP);
#if 1
	world_position PlayerLeft = NewPlayerP;
	PlayerLeft.Offset.X -= 0.5f * Entity->Width;
	PlayerLeft = ReCannonicalizePosition(World, PlayerLeft);

	world_position PlayerRight = NewPlayerP;
	PlayerRight.Offset.X += 0.5f * Entity->Width;
	PlayerRight = ReCannonicalizePosition(World, PlayerRight);

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

	bool32 Collided = false;
	world_position ColP = {};

	if (!IsTileMapPointEmpty(World, NewPlayerP)) {

		ColP = NewPlayerP;
		Collided = true;

	}
	if (!IsTileMapPointEmpty(World, PlayerLeft)) {

		ColP = PlayerLeft;
		Collided = true;
	}
	if (!IsTileMapPointEmpty(World, PlayerRight)) {

		ColP = PlayerRight;
		Collided = true;
	}

	if (Collided) {

		V2 r = { 0,0 };
		if (ColP.AbsTileX < Entity->P.AbsTileX) {

			r = V2{ 1,0 };
		}
		if (ColP.AbsTileX > Entity->P.AbsTileX) {
			r = V2{ -1,0 };
		}
		if (ColP.AbsTileY < Entity->P.AbsTileY) {
			r = V2{ 0,1 };
		}
		if (ColP.AbsTileY > Entity->P.AbsTileY) {
			r = V2{ 0,-1 };
		}
		//Bounce of the y axis , reduce this to along the wall by multiplying by 1
		Entity->dP = Entity->dP - 1 * Inner(Entity->dP, r) * r;

	}
	else
	{

		Entity->P = NewPlayerP;
	}
#else
	uint32 MinTileX = 0;
	uint32 MinTileY = 0;
	uint32 OnePastMaxTileY = 0;
	uint32 OnePastMaxTileX = 0;
	uint32 AbsTileZ = Entity->P.AbsTileZ;
	world_position BestPlayerP = Entity->P;
	real32 BestDistanceSq = LengthSq(PlayerDelta);

	for (uint32 AbsTileY = MinTileY;
		AbsTileY != OnePastMaxTileY;
		++AbsTileY) {

		for (uint32 AbsTileX = MinTileX;
			AbsTileX != OnePastMaxTileX;
			++AbsTileX) {

			world_position TestTileP = CenteredTilePoint(AbsTileX, AbsTileY, AbsTileZ);
			uint32 TileValue = GetTileValue(World, TestTileP);
			if (IsTileValueEmpty(TileValue)) {

				V2 MinCorner = -0.5f * V2{ World->TileSideInMeters,World->TileSideInMeters };
				V2 MaxCorner = 0.5f * V2{ World->TileSideInMeters,World->TileSideInMeters };

				tile_map_difference RelNewPlayerP = Subtract(World, &TestTileP, &NewPlayerP);
				V2 TestP = ClosestPointInRectangle(MinCorner, MaxCorner, RelNewPlayerP);
				if (...) {

				}
			}

		}

	}
#endif	
	//TODO : Change later for more complex time based way, sample offset
	if (!AreOnSameTile(&OldPlayerP, &Entity->P)) {

		uint32 NewTileValue = GetTileValue(World, Entity->P);
		if (NewTileValue == 3) {
			++Entity->P.AbsTileZ;

		}
		else if (NewTileValue == 4) {
			--Entity->P.AbsTileZ;
		}
	}

			if(Entity->dP.X == 0.0f && (Entity->dP.Y == 0.0f)){

			}
			else if(AbsoluteValue(Entity->dP.X) > AbsoluteValue(Entity->dP.Y)){

				if(Entity->dP.X > 0){
					
					Entity->FacingDirection = 0;
				}else{
					Entity->FacingDirection = 2;
				}

			}else {
	
				if(Entity->dP.Y > 0){
					
					Entity->FacingDirection = 1;
				}else{
					Entity->FacingDirection = 3;
				}

	}
	

}
#endif

internal uint32
AddEntity(game_state* GameState) {

	
	uint32 EntityIndex = GameState->EntityCount++;
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity* Entity = &GameState->Entities[EntityIndex];
	*Entity = {};

	return (EntityIndex);

}


extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{

	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;

	if(!Memory->IsInitialized){

		//So that the 0th index is not used
		uint32 NullEntityIndex = AddEntity(GameState);

		GameState->Backdrop = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_background.bmp");//"data/test/structured_art.bmp");
		
		hero_bitmaps * Bitmap;

		Bitmap = GameState->HeroBitmaps;

		//right
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_right_face.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_right_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Right_Torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		//back
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_back_head.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_back_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Back_Torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		//left
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_left_head.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_left_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Left_Torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;

		//front
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_head_new.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;
		

		GameState->CameraP.AbsTileX = 17/2;
		GameState->CameraP.AbsTileY = 9/2;
		//Entity->P.OffsetX = 5.0f;
		//Entity->P.OffsetY = 5.0f;


		
		InitailizeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
						(uint8 * )Memory->PermanentStorage + sizeof(game_state));


		GameState->World = PushStruct(&GameState-> WorldArena, world);
		world * World = GameState->World;
		World->World = PushStruct(&GameState-> WorldArena,world);

		world * World = World->World;

		World->ChunkShift = 4;
		World->ChunkMask = (1 << World->ChunkShift)-1;
		World->ChunkDim = (1 << World->ChunkShift);

		World->TileChunkCountX = 128;
		World->TileChunkCountY = 128;
		World->TileChunkCountZ = 2;

		World->TileChunks = PushArray(&GameState->WorldArena,
									   World->TileChunkCountX*
									   World->TileChunkCountY*
									   World->TileChunkCountZ,
									   world_chunk);
		

		World->TileSideInMeters = 1.4f;// this like a calibration value for our world
		
		uint32 RandomNumberIndex = 0 ;

		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;
		uint32 ScreenX =0;		
		uint32 ScreenY =0;
		//This is the valid declaration
		uint32 AbsTileZ =0;

		bool32 DoorLeft = false;
		bool32 DoorRight = false;
		bool32 DoorTop = false;
		bool32 DoorBottom = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;

		for(uint32 ScreenIndex =0;
			ScreenIndex < 100;
			++ScreenIndex){

		//Mode operation is being used to get basically 3 valid output
		Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));
		uint32 RandomChoice;
		if(DoorUp ||DoorDown){
		RandomChoice = RandomNumberTable[RandomNumberIndex++]%2;
		}
		else{
			RandomChoice = RandomNumberTable[RandomNumberIndex++]%3;
		}

		bool32 CreatedZDoor = false;
		if(RandomChoice == 2){
			CreatedZDoor = true;
			if(AbsTileZ == 0 ){
					DoorUp = true;
				}else{
					DoorDown = true;
				}
			
		}else if(RandomChoice == 1){
			DoorRight = true;

		}else{
			DoorTop = true;
		}

				for(uint32 TileY = 0;
					TileY < TilesPerHeight;
					++TileY){

					for(uint32 TileX = 0 ;
						TileX < TilesPerWidth;
						++TileX)
					{

						uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
						uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;
						



						//Making a boundary wall
						uint32 TileValue = 1;

						if(( TileX == 0 ) && (!DoorLeft || ( TileY != (TilesPerHeight/2)))){
							TileValue =2;
						}
						if(( TileX == (TilesPerWidth -1) ) && (!DoorRight|| ( TileY != (TilesPerHeight/2)))){
							TileValue =2;
						}

						if(( TileY == 0 ) && (!DoorBottom || (TileX != (TilesPerWidth/2)))){
							
							TileValue = 2;
						}
						if(( TileY == (TilesPerHeight -1) ) && (!DoorTop || (TileX != (TilesPerWidth/2)))){
							
							TileValue = 2;
						}
						if((TileX == 10)&&(TileY == 6)){

								if(DoorUp){
									TileValue = 3;
								}
								if(DoorDown){
									TileValue = 4;
								}

						}

						SetTileValue(&GameState->WorldArena, World->World,AbsTileX, AbsTileY,AbsTileZ,
							TileValue);
					}
			}

		DoorLeft = DoorRight;
		DoorBottom = DoorTop;
		
		if(CreatedZDoor){

			DoorDown = !DoorDown;
			DoorUp = !DoorUp;


		}else{

			DoorDown = false;
			DoorUp = false;

		}

		DoorRight = false;
		DoorTop = false;


			if(RandomChoice == 2){
				if(AbsTileZ == 0 ){
					AbsTileZ = 1;
				}else{
					AbsTileZ = 0;
				}
			}else if(RandomChoice == 1){
				ScreenX += 1;
			}
			else{

				ScreenY += 1;
			}
		}

	Memory->IsInitialized = true;

	}

	world * World = GameState->World;
	world * World = World->World;

	

	int32 TileSideInPixels = 60;
	real32 MetersToPixels = (real32)TileSideInPixels/(real32)World->TileSideInMeters;

	real32 LowerLeftX = -(real32)TileSideInPixels/2;
	real32 LowerLeftY = (real32)Buffer->Height;

	

 	for(int ControllerIndex = 0 ;
 		ControllerIndex < ArrayCount(Input->Controllers);
 			++ControllerIndex)
	{
		game_controller_input * Controller = GetController(Input,ControllerIndex);
		entity* ControllingEntity = GetEntity(GameState, GameState->PlayerIndexForController[ControllerIndex]);

		if (ControllingEntity) {

			V2 ddP = {};

			if (Controller->IsAnalog) {
				//digital controller = Keys
				ddP = V2{ Controller->StickAverageX, Controller->StickAverageY };
			}
			else {

				if (Controller->MoveUp.EndedDown) {

					ddP.Y = 1.0f;
				}
				if (Controller->MoveDown.EndedDown) {

					ddP.Y = -1.0f;

				}
				if (Controller->MoveLeft.EndedDown) {

					ddP.X = -1.0f;
				}
				if (Controller->MoveRight.EndedDown) {

					ddP.X = 1.0f;
				}

			}
			MovePlayer(GameState, ControllingEntity, Input->dtForFrame, ddP);

		}
		else {

			if (Controller->Start.EndedDown) {

				uint32 EntityIndex = AddEntity(GameState);
				ControllingEntity = GetEntity(GameState, EntityIndex);
				InitializePlayer(ControllingEntity);
				GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
			}

			
		}

	}

			entity* CameraFollowingEntity = GetEntity(GameState, GameState->CameraFollowingEntityIndex);
			if (CameraFollowingEntity) {
				GameState->CameraP.AbsTileZ = CameraFollowingEntity->P.AbsTileZ;
			

				tile_map_difference Diff = Subtract(World, &CameraFollowingEntity->P, &GameState->CameraP);

				if(Diff.dXY.X >(9.0f*World->TileSideInMeters)){
					GameState->CameraP.AbsTileX += 17;
				}
				if(Diff.dXY.X < -(9.0f*World->TileSideInMeters)){
					GameState->CameraP.AbsTileX -= 17;
				}
				if(Diff.dXY.Y >(5.0f*World->TileSideInMeters)){
					GameState->CameraP.AbsTileY += 9;
				}
				if(Diff.dXY.Y < -(5.0f*World->TileSideInMeters)){
					GameState->CameraP.AbsTileY -= 9;
				}

			}
	//For Debug purposes to display Gradient
	//RenderGrdaient(Buffer,BlueOffset, GreenOffset);
	
	DrawBitmap(Buffer, &GameState->Backdrop,0,0);

	real32 ScreenCenterX = 0.5f *(real32)Buffer->Width ;
	real32 ScreenCenterY = 0.5f* (real32)Buffer->Height;

	for(int32 RelRow = -10 ; RelRow < 10 ; ++RelRow){

		for(int32 RelColumn =  -20 ; RelColumn <  20 ; ++RelColumn){

			uint32 Column = GameState->CameraP.AbsTileX +RelColumn;
			uint32 Row = GameState->CameraP.AbsTileY + RelRow;
			uint32 TileID = GetTileValue(World, Column, Row,GameState->CameraP.AbsTileZ);

			if(TileID > 1){ //TileID == 0 meand there is empty space (cant move there)

				real32 Gray = 0.5f;
				if(TileID ==2 ){

					Gray = 1.0f;
				}
				if(TileID > 2){
					Gray = 0.25f;	
				}

				if((Column == GameState->CameraP.AbsTileX)&&
					(Row == GameState->CameraP.AbsTileY)){
					
					Gray = 0.0f;
				}

			
			//smooth scrolling effect
				V2 TileSide = {0.5f*TileSideInPixels, 0.5f*TileSideInPixels};
				V2 Cen = { ScreenCenterX - MetersToPixels*GameState->CameraP.Offset.X + ((real32)RelColumn)*TileSideInPixels,
						   ScreenCenterY + MetersToPixels*GameState->CameraP.Offset.Y - ((real32)RelRow)*TileSideInPixels};
				V2 Min = Cen - TileSide;
				V2 Max = Cen + TileSide;
				//real32 MinX = CenterX + ((real32)RelColumn)*World->TileSideInPixels;
				//real32 MinY = CenterY - ((real32)RelRow)*World->TileSideInPixels;


				DrawRectangle(Buffer,Min,Max,Gray,Gray,Gray);
			}	
		}

	}
	entity* Entity = GameState->Entities;
	for (uint32 EntityIndex = 0;
		EntityIndex < GameState->EntityCount;
		++EntityIndex, ++Entity) {

		if (Entity->Exists) {
			tile_map_difference Diff = Subtract(World, &Entity->P, &GameState->CameraP);

			real32 PlayerR = 1.0f;
			real32 PlayerG = 1.0f;
			real32 PlayerB = 0.0f;

			real32 PlayerGroundPointX = ScreenCenterX + MetersToPixels * Diff.dXY.X;
			real32 PlayerGroundPointY = ScreenCenterY - MetersToPixels * Diff.dXY.Y;
			real32 PlayerLeft = PlayerGroundPointX - (0.5f * MetersToPixels * Entity->Width);
			real32 PlayerTop = PlayerGroundPointY - (MetersToPixels * Entity->Height);

			/*DrawRectangle(Buffer,
						  PlayerLeft, PlayerTop,
						  PlayerLeft + MetersToPixels*Entity->Width,
						  PlayerTop + MetersToPixels*Entity->Height,
						  PlayerR,PlayerG,PlayerB);*/

			hero_bitmaps* HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
			DrawBitmap(Buffer, &HeroBitmaps->Torso, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
			DrawBitmap(Buffer, &HeroBitmaps->Cape, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
			DrawBitmap(Buffer, &HeroBitmaps->Head, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);

		}
	}
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
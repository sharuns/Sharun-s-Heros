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
ChangeEntityResidence(game_state* GameState, uint32  EntityIndex, entity_residence Residence) {
	if (Residence == EntityResidence_High) {

		if (GameState->EntityResidence[EntityIndex] != EntityResidence_High) {

			high_entity* EntityHigh = &GameState->HighEntities[EntityIndex];
			dormant_entity* EntityDormant = &GameState->DormantEntities[EntityIndex];

			tile_map_difference Diff = Subtract(GameState->World->TileMap,
												&EntityDormant->P, &GameState->CameraP);

			EntityHigh->P = Diff.dXY;
			EntityHigh->dP = V2{ 0, 0 };
			EntityHigh->AbsTileZ = EntityDormant->P.AbsTileZ;
			EntityHigh->FacingDirection = 0;


		}
	}
	GameState->EntityResidence[EntityIndex] = Residence;
}

/*
Method for returning an entity, these are stored in the game_state DS
This can hold as of now 256 entities, And we select the enitity based on the index passed
*/
inline entity
GetEntity(game_state * GameState, entity_residence Residence, uint32 Index)
{
	entity Entity = {};
	if((Index > 0 ) && (Index < GameState->EntityCount))
	{
		if (GameState->EntityResidence[Index] < Residence) 
		{
			ChangeEntityResidence(GameState, Index, Residence);
			Assert(GameState->EntityResidence[Index] >= Residence)
		}

		Entity.Residence = Residence;
		Entity.Dormant = &GameState->DormantEntities[Index];
		Entity.Low = &GameState->LowEntities[Index];
		Entity.High = &GameState->HighEntities[Index];
	}
	return (Entity);
}


/*
Called at the start to create our player
*/
internal void
InitializePlayer( game_state * GameState, uint32 EntityIndex){

	entity Entity = GetEntity(GameState, EntityResidence_Dormant, EntityIndex);

		Entity.Dormant->P.AbsTileX = 1;
		Entity.Dormant->P.AbsTileY = 3;
		Entity.Dormant->P.Offset_.X = 0;
		Entity.Dormant->P.Offset_.Y = 0;
		Entity.Dormant->Height = 0.5f;
		Entity.Dormant->Width = 1.0f;
		Entity.Dormant->Collides = true;


		ChangeEntityResidence(GameState, EntityIndex, EntityResidence_High);
		/*Getting the camera to initialise sort of 
		 And setting it to point to the player created as the cameraFollowingEntityIndex is 0 in the begining
		*/
		if (GetEntity(GameState, EntityResidence_Dormant, GameState->CameraFollowingEntityIndex).Residence ==
			EntityResidence_Nonexistant) {
			GameState->CameraFollowingEntityIndex = EntityIndex;
		}
}


internal uint32
AddEntity(game_state * GameState){

	uint32 EntityIndex = GameState->EntityCount++;
	Assert(GameState->EntityCount < ArrayCount(GameState->DormantEntities));
	Assert(GameState->EntityCount < ArrayCount(GameState->LowEntities));
	Assert(GameState->EntityCount < ArrayCount(GameState->HighEntities));

	GameState->EntityResidence[EntityIndex] = EntityResidence_Dormant;
	GameState->DormantEntities[EntityIndex] = {};
	GameState->LowEntities[EntityIndex] = {};
	GameState->HighEntities[EntityIndex] = {};

	return (EntityIndex);
}

/* Collision detection code for chekcing the point collision*/
internal bool32
TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY,
	     real32* tMin, real32 MinY, real32 MaxY)
{
	bool32 Hit = false;
	real32 tEpsilon = 0.001f; 
	if (PlayerDeltaX != 0.0f)
	{
		real32 tResult = (WallX - RelX) / PlayerDeltaX;
		real32 Y = RelY + tResult * PlayerDeltaY;

		if ((tResult >= 0.0f) && (*tMin > tResult))
		{
			if ((Y >= MinY) && (Y <= MaxY))
			{
				*tMin = Maximum(0.0f,tResult - tEpsilon);
				Hit = true;
			}
		}
	}
	return (Hit);
}

internal void
MovePlayer(game_state* GameState, entity Entity, real32 dt, V2 ddP) 
{


	tile_map* TileMap = GameState->World->TileMap;

	real32 ddPLength = LengthSq(ddP);
	if (ddPLength > 1.0f) {

		ddP *= (1.0f / SquareRoot(ddPLength));
	}

	//This is default acceleration value of the player
	real32 PlayerSpeed = 25.0f;// m/s^2
	ddP *= PlayerSpeed;

	ddP += -7.0f * Entity.High->dP;

	V2 OldPlayerP = Entity.High->P;
	V2 PlayerDelta = (0.5f * ddP * Square(dt) +
		Entity.High->dP * dt);
	// velocity equation v' = (at + v)
	Entity.High->dP = ddP * dt + Entity.High->dP;
	V2 NewPlayerP = OldPlayerP + PlayerDelta;

	//position equation p' = (1/2 * at^2 + Vt + p)
	/*collision detection : based on the Basic Minkowski collision detection*/



	real32 tRemaining = 1.0f;
	for (uint32 Iterations = 0;
		(Iterations < 4) && (tRemaining > 0.0f);
		++Iterations)
	{
		real32 tMin = 1.0f;
		V2 WallNormal = {};
		uint32 HitEntityIndex = 0;

		for (uint32 EntityIndex = 1;
			EntityIndex < GameState->EntityCount;
			++EntityIndex)
		{
			entity TestEntity = GetEntity(GameState, EntityResidence_High, EntityIndex);
			if(TestEntity.High != Entity.High)
			{
				if (TestEntity.Dormant->Collides)
				{
					real32 DiameterW = TestEntity.Dormant->Width + Entity.Dormant->Width;
					real32 DiameterH = TestEntity.Dormant->Height + Entity.Dormant->Height;
					V2 MinCorner = -0.5f * V2{ DiameterW, DiameterH };
					V2 MaxCorner = 0.5f * V2{ DiameterW, DiameterH };

					V2 Rel = Entity.High->P - TestEntity.High->P;

					if (TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
						&tMin, MinCorner.Y, MaxCorner.Y))
					{

						WallNormal = V2{ -1,0 };
						HitEntityIndex = EntityIndex;
					}

					if (TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
						&tMin, MinCorner.Y, MaxCorner.Y))
					{
						WallNormal = V2{ 1,0 };
						HitEntityIndex = EntityIndex;
					}

					if (TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
						&tMin, MinCorner.X, MaxCorner.X))
					{
						WallNormal = V2{ 0,-1 };
						HitEntityIndex = EntityIndex;
					}

					if (TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
						&tMin, MinCorner.X, MaxCorner.X))
					{
						WallNormal = V2{ 0,1 };
						HitEntityIndex = EntityIndex;
					}

				}
			}
		}
		//end of collision dectection

		Entity.High->P += tMin * PlayerDelta;
		if (HitEntityIndex) {

			Entity.High->dP = Entity.High->dP - 1 * Inner(Entity.High->dP, WallNormal) * WallNormal;
			PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
			tRemaining -= tMin * tRemaining;

			entity HitEntity = GetEntity(GameState, EntityResidence_Dormant, HitEntityIndex);
			Entity.High->AbsTileZ += HitEntity.Dormant->dAbsTileZ;
		}
		else {
			break;
		}
	
	}
	//position equation p' = (1/2 * at^2 + Vt + p)


			if(Entity.High->dP.X == 0.0f && (Entity.High->dP.Y == 0.0f)){

			}
			else if(AbsoluteValue(Entity.High->dP.X) > AbsoluteValue(Entity.High->dP.Y)){

				if(Entity.High->dP.X > 0){
					
					Entity.High->FacingDirection = 0;
				}else{
					Entity.High->FacingDirection = 2;
				}

			}else {
	
				if(Entity.High->dP.Y > 0){
					
					Entity.High->FacingDirection = 1;
				}else{
					Entity.High->FacingDirection = 3;
				}

			}

	Entity.Dormant->P = MapIntoTileSpace(GameState->World->TileMap,GameState->CameraP,Entity.High->P);
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
		//GameState->PlayerP.OffsetX = 5.0f;
		//GameState->PlayerP.OffsetY = 5.0f;


		
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
		TileMap->TileChunkCountZ = 2;

		TileMap->TileChunks = PushArray(&GameState->WorldArena,
									   TileMap->TileChunkCountX*
									   TileMap->TileChunkCountY*
									   TileMap->TileChunkCountZ,
									   tile_chunk);
		

		TileMap->TileSideInMeters = 1.4f;// this like a calibration value for our world
		
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

						SetTileValue(&GameState->WorldArena, World->TileMap,AbsTileX, AbsTileY,AbsTileZ,
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
	tile_map * TileMap = World->TileMap;

	

	int32 TileSideInPixels = 60;
	real32 MetersToPixels = (real32)TileSideInPixels/(real32)TileMap->TileSideInMeters;

	real32 LowerLeftX = -(real32)TileSideInPixels/2;
	real32 LowerLeftY = (real32)Buffer->Height;

	

 	for(int ControllerIndex = 0 ;
 		ControllerIndex < ArrayCount(Input->Controllers);
  			++ControllerIndex)
	{
		game_controller_input * Controller = GetController(Input,ControllerIndex);
		entity ControllingEntity  = GetEntity(GameState, EntityResidence_High, GameState->PlayerIndexForController[ControllerIndex]);
		if(ControllingEntity.Residence != EntityResidence_Nonexistant){

			V2 ddP ={};

			if(Controller->IsAnalog){

				ddP = V2{Controller->StickAverageX, Controller->StickAverageY};

			}else{
					//digital controller = Key

				if(Controller->MoveUp.EndedDown){

					ddP.Y = 1.0f;
				}
				if(Controller->MoveDown.EndedDown){
			
					ddP.Y = -1.0f;

				}
				if(Controller->MoveLeft.EndedDown){

					ddP.X = -1.0f;
				}
				if(Controller->MoveRight.EndedDown){
				
					ddP.X = 1.0f;
				}
			}
				MovePlayer(GameState,ControllingEntity,Input->dtForFrame,ddP);

		}else{

			if(Controller->Start.EndedDown){

				uint32 EntityIndex = AddEntity(GameState);
				InitializePlayer( GameState, EntityIndex);
				GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
			}
	}
}
	V2 EntityOffSetForFrame = {};
	entity CameraFollowingEntity = GetEntity(GameState, EntityResidence_High, GameState->CameraFollowingEntityIndex);
	if(CameraFollowingEntity.Residence != EntityResidence_Nonexistant){

		tile_map_position OldCameraP = GameState->CameraP;
	
		GameState->CameraP.AbsTileZ = CameraFollowingEntity.Dormant->P.AbsTileZ;
		
		if(CameraFollowingEntity.High->P.X  >(9.0f*TileMap->TileSideInMeters)){
			GameState->CameraP.AbsTileX += 17;
		}
		if(CameraFollowingEntity.High->P.X < -(9.0f*TileMap->TileSideInMeters)){
			GameState->CameraP.AbsTileX -= 17;
		}
		if(CameraFollowingEntity.High->P.Y >(5.0f*TileMap->TileSideInMeters)){
			GameState->CameraP.AbsTileY += 9;
		}
		if(CameraFollowingEntity.High->P.Y < -(5.0f*TileMap->TileSideInMeters)){
			GameState->CameraP.AbsTileY -= 9;
		}

		tile_map_difference dCameraP = Subtract(TileMap, &GameState->CameraP, &OldCameraP);
		EntityOffSetForFrame = -dCameraP.dXY;

	}
	
	DrawBitmap(Buffer, &GameState->Backdrop,0,0);

	real32 ScreenCenterX = 0.5f *(real32)Buffer->Width ;
	real32 ScreenCenterY = 0.5f* (real32)Buffer->Height;

	for(int32 RelRow = -10 ; RelRow < 10 ; ++RelRow){

		for(int32 RelColumn =  -20 ; RelColumn <  20 ; ++RelColumn){

			uint32 Column = GameState->CameraP.AbsTileX +RelColumn;
			uint32 Row = GameState->CameraP.AbsTileY + RelRow;
			uint32 TileID = GetTileValue(TileMap, Column, Row,GameState->CameraP.AbsTileZ);

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
				V2 Cen = { ScreenCenterX - MetersToPixels*GameState->CameraP.Offset_.X + ((real32)RelColumn)*TileSideInPixels,
						   ScreenCenterY + MetersToPixels*GameState->CameraP.Offset_.Y - ((real32)RelRow)*TileSideInPixels};
				V2 Min = Cen - 0.9f*TileSide;
				V2 Max = Cen + 0.9f *TileSide;


				DrawRectangle(Buffer,Min,Max,Gray,Gray,Gray);
			}	
		}

	}

	//Rendering code : Using the entity system
	for(uint32 EntityIndex = 0;
	EntityIndex < GameState->EntityCount;
	++EntityIndex){

	if(GameState->EntityResidence[EntityIndex] == EntityResidence_High)
	{
		high_entity* HighEntity = &GameState->HighEntities[EntityIndex];
		low_entity* LowEntity = &GameState->LowEntities[EntityIndex];
		dormant_entity* DormantEntity = &GameState->DormantEntities[EntityIndex];

		HighEntity->P += EntityOffSetForFrame;

		real32 PlayerR = 1.0f;
		real32 PlayerG = 1.0f;
		real32 PlayerB = 0.0f;
		real32 PlayerGroundPointX = ScreenCenterX + MetersToPixels*HighEntity->P.X;
		real32 PlayerGroundPointY = ScreenCenterY - MetersToPixels*HighEntity->P.Y;
		V2 PlayerLeftTop = {PlayerGroundPointX - 0.5f*MetersToPixels*DormantEntity->Width,
							PlayerGroundPointY - 0.5f*MetersToPixels*DormantEntity->Height};
		V2 EntityWidthHeight = { DormantEntity->Width, DormantEntity->Height };
		DrawRectangle(Buffer,
					  PlayerLeftTop,
					  PlayerLeftTop + MetersToPixels*EntityWidthHeight,
					  PlayerR,PlayerG,PlayerB);

		hero_bitmaps * HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];
		DrawBitmap(Buffer, &HeroBitmaps->Torso, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX,HeroBitmaps->AlignY);
		DrawBitmap(Buffer, &HeroBitmaps->Cape, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX,HeroBitmaps->AlignY);
		DrawBitmap(Buffer, &HeroBitmaps->Head, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX,HeroBitmaps->AlignY);
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
/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/
#include "SharunMade_Platform.h"
#include "SharunMade.h"
#include "SharunMadeWorld.cpp"
#include "RandomNo.h"
#include "SharunMade_sim_region.cpp"
#include "SharunMade_Entity.cpp"

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
	v2 vMin, v2 vMax,
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
			real32 CAlpha = 1.0f)
{

	int32 MinX = RoundReal32ToInt32(RealX);
	int32 MinY = RoundReal32ToInt32(RealY);
	int32 MaxX = MinX + Bitmap->Width;
	int32 MaxY = MinY + Bitmap->Height;

	//Clipping the rectangle so that we are drawing in the valid buffer section
	int32 SourceOffsetX = 0;
	if (MinX < 0)
	{
		SourceOffsetX = -MinX;
		MinX = 0;
	}
	int32 SourceOffsetY = 0;
	if (MinY < 0)
	{
		SourceOffsetY = -MinY;
		MinY = 0;
	}
	if (MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if (MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	uint32 * SourceRow  = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height -1);
	SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;
	uint8 * DestRow = ((uint8 *)Buffer->Memory + 
						MinX*Buffer->BytesPerPixel + 
						MinY*Buffer->Pitch);
	for(int32 Y = MinY ;
		Y < MaxY;
		++Y)
	{

		uint32 *Dest = (uint32 *)DestRow;
		uint32 * Source = SourceRow;
		for(int32 X = MinX ; 
			X < MaxX;
			++X)
		{

			//################linear alpha blending################

			real32 A = (real32)((*Source >> 24) & 0xFF)/255.0f;
			A *= CAlpha;
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

struct add_low_entity_result
{
	low_entity* Low;
	uint32 LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state* GameState, entity_type Type, world_position P)
{
	Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
	uint32 EntityIndex = GameState->LowEntityCount++;

	low_entity* EntityLow = GameState->LowEntities + EntityIndex;
	*EntityLow = {};
	 EntityLow->Sim.Type = Type;
	 EntityLow->Sim.Collision = GameState->NullCollision;
	 EntityLow->P = NullPosition();

	
	ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, P);
	
	add_low_entity_result Result;
	Result.Low = EntityLow;
	Result.LowIndex = EntityIndex;

	return (Result);
}

internal add_low_entity_result
AddGroundEntity(game_state* GameState, entity_type Type, world_position P, sim_entity_collision_volume_group *Collision)
{
	add_low_entity_result Entity = AddLowEntity(GameState, Type, P);
	Entity.Low->Sim.Collision = Collision;
	return (Entity);
}

internal add_low_entity_result
AddStandardRoom(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Space, P, GameState->StandardRoomCollision);

	AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);

	return (Entity);
}


internal add_low_entity_result
AddWall(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Wall,P, GameState->WallCollision);

	AddFlags(&Entity.Low->Sim, EntityFlag_Collides);

	return (Entity);
}

internal add_low_entity_result
AddStair(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Stairwell, P, GameState->StairwellCollision);

	AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
	Entity.Low->Sim.WalkableDim = (Entity.Low->Sim.Collision->TotalVolume.Dim.XY);
	Entity.Low->Sim.WalkableHeight = GameState->World->TileDepthInMeters;

	return (Entity);
}


internal void
InitHitPoints(low_entity * EntityLow, uint32 HitPointCount)
{
	Assert(HitPointCount <= ArrayCount(EntityLow->Sim.HitPoint));
	EntityLow->Sim.HitPointMax = HitPointCount;
	for (uint32 HitPointIndex = 0;
		HitPointIndex < EntityLow->Sim.HitPointMax;
		++HitPointIndex)
	{
		hit_point* HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
		HitPoint->Flags = 0;
		HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
	}
}

internal add_low_entity_result
AddSword(game_state* GameState)
{
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());
	Entity.Low->Sim.Collision = GameState->SwordCollision;

	AddFlags(&Entity.Low->Sim,EntityFlag_Moveable);

	return (Entity);
}

internal add_low_entity_result
AddPlayer( game_state * GameState)
{
	world_position P = GameState->CameraP;;
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Hero, P, GameState->PlayerCollision);
	AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);

	InitHitPoints(Entity.Low, 3);

	add_low_entity_result Sword = AddSword(GameState);
	Entity.Low->Sim.Sword.Index = Sword.LowIndex;

	if (GameState->CameraFollowingEntityIndex ==0)
	{
		GameState->CameraFollowingEntityIndex = Entity.LowIndex;
	}
	
	return (Entity);
}

internal add_low_entity_result
AddMonster(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Monster, P, GameState->MonsterCollision);

	AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);

	InitHitPoints(Entity.Low, 3);

	return (Entity);
}

internal add_low_entity_result
AddFamiliar(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddGroundEntity(GameState, EntityType_Familiar, P, GameState->FamiliarCollision);

	AddFlags(&Entity.Low->Sim, EntityFlag_NoCollide | EntityFlag_Moveable );

	return (Entity);
}

internal void
PushPiece(entity_visible_piece_group* Group, loaded_bitmap* Bitmap,
	v2 Offset, real32 OffsetZ, v2 Align, v2 Dim, v4 Color, real32 EntityZC)
{
	Assert(Group->PieceCount < ArrayCount(Group->Pieces));
	entity_visible_piece* Piece = Group->Pieces + Group->PieceCount++;
	Piece->Bitmap = Bitmap;
	Piece->Offset = Group->GameState->MetersToPixels * V2(Offset.X, -Offset.Y) - Align;
	Piece->OffsetZ = Group->GameState->MetersToPixels * OffsetZ;
	Piece->A = Color.A;
	Piece->EntityZC = EntityZC;
	Piece->R = Color.R;
	Piece->G = Color.G;
	Piece->B = Color.B;
	Piece->Dim = Dim;
}

internal void
PushBitmap(entity_visible_piece_group* Group,loaded_bitmap * Bitmap,
		v2 Offset, real32 OffsetZ, v2 Align, real32 Alpha = 1.0f, real32 EntityZC = 1.0f)
{
	PushPiece(Group, Bitmap, Offset, OffsetZ, Align,V2(0,0), V4(1.0f, 1.0f, 1.0f, Alpha), EntityZC);
}

internal void
PushRect(entity_visible_piece_group* Group, v2 Offset, real32 OffsetZ,
		v2 Dim, v4 Color, real32 EntityZC = 1.0f)
{
	PushPiece(Group, 0, Offset, OffsetZ, V2(0,0),Dim, Color, EntityZC);
}

internal void
PushRectOutline(entity_visible_piece_group* Group, v2 Offset, real32 OffsetZ,
	v2 Dim, v4 Color, real32 EntityZC = 1.0f)
{
	real32 Thickness = 0.1f;
	PushPiece(Group, 0, Offset - V2(0, 0.5f*Dim.Y), OffsetZ, V2(0,0), V2(Dim.X,Thickness), Color, EntityZC);
	PushPiece(Group, 0, Offset + V2(0, 0.5f * Dim.Y), OffsetZ, V2(0, 0), V2(Dim.X, Thickness), Color, EntityZC);

	PushPiece(Group, 0, Offset - V2(0.5f*Dim.X, 0), OffsetZ, V2(0, 0), V2(Thickness, Dim.Y), Color, EntityZC);
	PushPiece(Group, 0, Offset + V2(0.5f*Dim.X, 0), OffsetZ, V2(0, 0), V2(Thickness, Dim.Y), Color, EntityZC);


}

internal void
DrawHitPoints(sim_entity * Entity, entity_visible_piece_group * PieceGroup)
{
	if (Entity->HitPointMax >= 1)
	{
		v2 HealthDim = { 0.2f,0.2f };
		real32 SpacingX = 1.5f * HealthDim.X;
		v2 HitP = { -0.5f * (Entity->HitPointMax - 1) * SpacingX, -0.25f };
		v2 dHitP = { SpacingX, 0.0f };
		for (uint32 HealthIndex = 0;
			HealthIndex < Entity->HitPointMax;
			++HealthIndex)
		{
			hit_point* HitPoint = Entity->HitPoint + HealthIndex;
			v4 Color = { 1.0f, 0.0f, 0.0f, 1.0f };
			if (HitPoint->FilledAmount == 0)
			{
				Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
			}

			PushRect(PieceGroup, HitP, 0, HealthDim, Color, 0.0f);
			HitP += dHitP;
		}
	}
}

internal void
ClearCollisionRulesFor(game_state* GameState, uint32 StorageIndex)
{
	for (uint32 HashBucket = 0;
		HashBucket < ArrayCount(GameState->CollisionRuleHash);
		++HashBucket)
	{
		for (pairwise_collision_rule** Rule = &GameState->CollisionRuleHash[HashBucket];
			*Rule;
			)
		{
			if (((*Rule)->StorageIndexA == StorageIndex) ||
				((*Rule)->StorageIndexB == StorageIndex))
			{
				pairwise_collision_rule* RemoveRule = *Rule;
				*Rule = (*Rule)->NextInHash;

				RemoveRule->NextInHash = GameState->FirstFreeCollisionRule;
				GameState->FirstFreeCollisionRule = RemoveRule;
			}
			else
			{
				Rule = &(*Rule)->NextInHash;
			}
		}
	}

}


internal bool32
RemoveCollisionRule(game_state* GameState, uint32 StorageIndexA, uint32 StorageIndexB)
{
}

internal void
AddCollisionRule(game_state* GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 CanCollide)
{
	bool32 Added = false;
	bool32 Result = false;
	if (StorageIndexA > StorageIndexB)
	{
		uint32  Temp = StorageIndexA;
		StorageIndexA = StorageIndexB;
		StorageIndexB = Temp;
	}

	pairwise_collision_rule* Found = 0;
	uint32 HashBucket = StorageIndexA & (ArrayCount(GameState->CollisionRuleHash) - 1);
	for (pairwise_collision_rule* Rule = GameState->CollisionRuleHash[HashBucket];
		Rule;
		Rule = Rule->NextInHash)
	{
		if ((Rule->StorageIndexA == StorageIndexA) &&
			(Rule->StorageIndexB == StorageIndexB))
		{
			Found = Rule;
			break;
		}
	}

	if (!Found)
	{
		Found = GameState->FirstFreeCollisionRule;
		if (Found)
		{
			GameState->FirstFreeCollisionRule = Found->NextInHash;
		}
		else
		{
			Found = PushStruct(&GameState->WorldArena, pairwise_collision_rule);
		}
		
		Found->NextInHash = GameState->CollisionRuleHash[HashBucket];
		GameState->CollisionRuleHash[HashBucket] = Found;
		Added = true;
	}
	if (Found)
	{
		Found->StorageIndexA = StorageIndexA;
		Found->StorageIndexB = StorageIndexB;
		Found->CanCollide = CanCollide;
	}

	//return (Added);
}

sim_entity_collision_volume_group*
MakeSimpleGroundedCollision(game_state * GameState, real32 DimX, real32 DimY, real32 DimZ)
{
	sim_entity_collision_volume_group* Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
	Group->VolumeCount = 1;
	Group->Volumes = PushArray(&GameState->WorldArena, Group->VolumeCount, sim_entity_collision_volume);
	Group->TotalVolume.OffsetP = V3(0, 0, 0.5f * DimZ);
	Group->TotalVolume.Dim = V3(DimX, DimY, DimZ);
	Group->Volumes[0] = Group->TotalVolume;

	return (Group);
}

sim_entity_collision_volume_group*
MakeNullCollision(game_state* GameState)
{
	sim_entity_collision_volume_group* Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
	Group->VolumeCount = 0;
	Group->Volumes = 0;
	Group->TotalVolume.OffsetP = V3(0, 0, 0);
	Group->TotalVolume.Dim = V3(0, 0, 0);
	
	return (Group);
}

extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;

	if(!Memory->IsInitialized){

		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;
		//So that the 0th index is not used
		InitailizeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
			(uint8*)Memory->PermanentStorage + sizeof(game_state));
		
		AddLowEntity(GameState, Entity_Null, NullPosition());
		GameState->World = PushStruct(&GameState->WorldArena, world);
		world* World = GameState->World;
		InitializeWorld(World, 1.4f, 3.0f);//1.4f is the tiles in meters size

		int32 TilesSideInPixels = 60;
		GameState->MetersToPixels = (real32)TilesSideInPixels / (real32)World->TileSideInMeters;

		GameState->NullCollision = MakeNullCollision(GameState);
		GameState->SwordCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.1f);
		GameState->StairwellCollision = MakeSimpleGroundedCollision(GameState, GameState->World->TileSideInMeters,
																2.0f * GameState->World->TileSideInMeters,
																GameState->World->TileDepthInMeters);
		GameState->PlayerCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 1.2f);
		GameState->MonsterCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
		GameState->WallCollision = MakeSimpleGroundedCollision(GameState,
																GameState->World->TileSideInMeters,
																GameState->World->TileSideInMeters,
																GameState->World->TileDepthInMeters);
		GameState->StandardRoomCollision = MakeSimpleGroundedCollision(GameState,
																TilesPerWidth*GameState->World->TileSideInMeters,
																TilesPerHeight*GameState->World->TileSideInMeters,
																0.9f*GameState->World->TileDepthInMeters);
		GameState->FamiliarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);

		GameState->Backdrop = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_background.bmp");
		GameState->Shadow = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_shadow.bmp");
		GameState->Tree = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test2/tree00.bmp");
		GameState->Sword = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test2/rock03.bmp");
		GameState->Stairwell = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test2/rock02.bmp");
		hero_bitmaps * Bitmap;
		Bitmap = GameState->HeroBitmaps;

		//right
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_right_face.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_right_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Right_Torso.bmp");
		Bitmap->Align = V2(72,182);
		++Bitmap;

		//back
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_back_head.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_back_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Back_Torso.bmp");
		Bitmap->Align = V2(72, 182);
		++Bitmap;

		//left
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_left_head.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_left_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_Left_Torso.bmp");
		Bitmap->Align = V2(72, 182);
		++Bitmap;

		//front
		Bitmap->Head  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_head_new.bmp");
		//Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test/bat.bmp");
		Bitmap->Cape  = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_Cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_front_torso.bmp");
		Bitmap->Align = V2(72, 182);
		++Bitmap;
		
		uint32 RandomNumberIndex = 0 ;
		
		
		/*
		* [ssk] : We start in the center of the world
		Sides of the world are INT16_MAX
		-------------------------------------
		|                                    |
		|                                    |
		|                                    |
		|                .                   |
		|                                    |
		|                                    |
		|                                    |
		|                                    |
		-------------------------------------
		
		For now for the ease of development we are again starting from 0,0
		*/
		uint32 ScreenBaseX = 0;//(INT16_MAX / TilesPerWidth) /2;
		uint32 ScreenBaseY = 0;// (INT16_MAX / TilesPerHeight) / 2;
		uint32 ScreenBaseZ = 0;// (INT16_MAX / 2);

		uint32 ScreenX = ScreenBaseX;		
		uint32 ScreenY = ScreenBaseY;
		uint32 AbsTileZ = ScreenBaseZ;

		bool32 DoorLeft = false;
		bool32 DoorRight = false;
		bool32 DoorTop = false;
		bool32 DoorBottom = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;

		//[ssk] : For structuring the different screens : At the moment only drawing 2 of them 

		for(uint32 ScreenIndex =0;
			ScreenIndex < NUMSCREENS;
			++ScreenIndex)
		{

			/*[ssk] : Mode operation is being used to get basically 3 valid output
			 Valid outputs being 0,1 and 2 
			so we asign 2 for door up/ down , 0 for Left door and 1 for Right door*/

			Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));
			uint32 RandomChoice;
			
		if(DoorUp ||DoorDown)
			{
			RandomChoice = RandomNumberTable[RandomNumberIndex++]%2;
			}
			else
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++]%3;
			}
			bool32 CreatedZDoor = false;
			if(RandomChoice == 2){
				CreatedZDoor = true;
				if(AbsTileZ == ScreenBaseZ ){
						DoorUp = true;
					}else{
						DoorDown = true;
					}
				
			}else if(RandomChoice == 1){
				DoorRight = true;

			}else{
				DoorTop = true;
			}

			AddStandardRoom(GameState,
				ScreenX * TilesPerWidth + TilesPerWidth/2,
				ScreenY * TilesPerHeight + TilesPerHeight/2,
				AbsTileZ);

			//[ssk] : looping through all the TilesPerWidth X TilesPerHeight (17X9 = 154 )
			for(uint32 TileY = 0;
				TileY < TilesPerHeight;
				++TileY)
			{

				for(uint32 TileX = 0 ;
					TileX < TilesPerWidth;
					++TileX)
				{
					/*[ssk] : Tile value can be 1,2,3,4
						1 is right or left or empty space ??
						2 is wall
						3 is door up
						4 is door down
					
					*/
					uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
					uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;
					//[ssk] : Making a boundary wall
					bool32 ShouldBeDoor = false;

					if(( TileX == 0 ) && (!DoorLeft || ( TileY != (TilesPerHeight/2))))
					{
						ShouldBeDoor = true;
					}
					if(( TileX == (TilesPerWidth -1) ) && (!DoorRight|| ( TileY != (TilesPerHeight/2))))
					{
						ShouldBeDoor = true;
					}
					if(( TileY == 0 ) && (!DoorBottom || (TileX != (TilesPerWidth/2))))
					{	
						ShouldBeDoor = true;
					}
					if(( TileY == (TilesPerHeight -1) ) && (!DoorTop || (TileX != (TilesPerWidth/2))))
					{	
						ShouldBeDoor = true;
					}

					if (ShouldBeDoor)
					{
						if(ScreenIndex == 0)
						AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
					}
					else if (CreatedZDoor)
					{
						if ((TileX == 10) && (TileY == 5))
						{
							AddStair(GameState, AbsTileX, AbsTileY, DoorDown ? AbsTileZ -1 : AbsTileZ);
						}
					}
					//[ssk] : Add wall entity based on the tile value 52 wall tiles ?
					
				}//[ssk] : End of for loop :  TilesPerWidth

			}//[ssk] : End of for loop : TilesPerHeight

			DoorLeft = DoorRight;
			DoorBottom = DoorTop;
			
			if(CreatedZDoor)
			{
				DoorDown = !DoorDown;
				DoorUp = !DoorUp;

			}
			else
			{
				DoorDown = false;
				DoorUp = false;
			}

			DoorRight = false;
			DoorTop = false;


			if(RandomChoice == 2)
			{
				if(AbsTileZ == ScreenBaseZ )
				{
					AbsTileZ = ScreenBaseZ + 1;
				}
				else
				{
					AbsTileZ = ScreenBaseZ;
				}
			}
			else if(RandomChoice == 1)
			{
				ScreenX += 1;
			}
			else{

				ScreenY += 1;
			}
		}//[ssk] : End of for loop : NUMSCREENS

		//[ssk] : Setting up Camera Starting position, Almost to the center of the world where the screen is
		world_position NewCameraP = {};

		uint32 CameraTileX = ScreenBaseX * TilesPerWidth + (17 / 2);
		uint32 CameraTileY = ScreenBaseY * TilesPerHeight + (9 / 2);
		uint32 CameraTileZ = ScreenBaseZ;
		NewCameraP = ChunkPositionFromTilePosition(GameState->World, CameraTileX, CameraTileY, CameraTileZ);

		GameState->CameraP = NewCameraP;
		
		AddMonster(GameState, CameraTileX - 3, CameraTileY + 2, CameraTileZ);

		for (int FamiliarIndex = 0;
			FamiliarIndex < 1;
			++FamiliarIndex)
		{
			int32 FamiliarOffsetX = (RandomNumberTable[RandomNumberIndex++] % 10) - 7;
			int32 FamiliarOffsetY = (RandomNumberTable[RandomNumberIndex++] % 10) - 3;
			if ((FamiliarOffsetX != 0) ||
				(FamiliarOffsetY != 0))
			{
				AddFamiliar(GameState, CameraTileX + FamiliarOffsetX, CameraTileY + FamiliarOffsetY, CameraTileZ);
			}
		}

		Memory->IsInitialized = true;

	}

	world * World = GameState->World;
	//world * World = World->World;

	int32 TileSideInPixels = 60;

	real32 MetersToPixels = (real32)TileSideInPixels / (real32)World->TileSideInMeters;


	//[ssk] : Controller handling And player movement
 	for(int ControllerIndex = 0 ;
 		ControllerIndex < ArrayCount(Input->Controllers);
  			++ControllerIndex)
	{
		game_controller_input * Controller = GetController(Input,ControllerIndex);
		controlled_hero * ConHero = GameState->ControlledHeros + ControllerIndex;
		if (ConHero->EntityIndex == 0)
		{
			//[ssk] : On hiting space we add a player entity
			if (Controller->Start.EndedDown)
			{
				*ConHero = {};
				ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
			}
		}
		else
		{
			ConHero->dZ = 0.0f;
			ConHero->ddP = {};
			ConHero->dSword = {};

			if (Controller->IsAnalog)
			{
				ConHero->ddP = v2{ Controller->StickAverageX, Controller->StickAverageY };
			}
			else
			{
				//digital controller = Key
				if (Controller->MoveUp.EndedDown)
				{
					ConHero->ddP.Y = 1.0f;
				}
				if (Controller->MoveDown.EndedDown) 
				{
					ConHero->ddP.Y = -1.0f;
				}
				if (Controller->MoveLeft.EndedDown) 
				{
					ConHero->ddP.X = -1.0f;
				}
				if (Controller->MoveRight.EndedDown) 
				{
					ConHero->ddP.X = 1.0f;
				}
			}
			if (Controller->Start.EndedDown)
			{
				ConHero->dZ = 3.0f;
			}

			ConHero->dSword = {};

			if (Controller->ActionUp.EndedDown)
			{
				ConHero->dSword = V2(0.0f, 1.0f);
			}
			if (Controller->ActionDown.EndedDown)
			{
				ConHero->dSword = V2(0.0f, -1.0f);
			}
			if (Controller->ActionLeft.EndedDown)
			{
				ConHero->dSword = V2(-1.0f, 0.0f);
			}
			if (Controller->ActionRight.EndedDown)
			{
				ConHero->dSword = V2(1.0f, 0.0f);
			}
		}
	}//[ssk] : End of for loop for controller


	uint32 TileSpanX = 17 * 3;
	uint32 TileSpanY = 9 * 3;
	uint32 TileSpanZ = 1;
	rectangle3 CameraBounds = RectCenterDim(V3(0, 0, 0),
											V3((real32)TileSpanX,
											(real32)TileSpanY,
											(real32)TileSpanZ)*World->TileSideInMeters);

	memory_arena SimArena;
	InitailizeArena(&SimArena, Memory->TransientStorageSize, Memory->TransientStorage);
	sim_region* SimRegion = BeginSim(&SimArena, GameState, GameState->World, GameState->CameraP, CameraBounds, Input->dtForFrame);
	//BeginSim(memory_arena *SimArena,game_state *GameState, world * World ,world_position Origin, rectangle2 Bounds)
				
	
	//Rendering code : Using the entity system
#if 1 
	DrawRectangle(Buffer, V2(0.0f, 0.0f), V2((real32)Buffer->Width, (real32)Buffer->Height), 0.5f, 0.5f, 0.5f);
#else
	DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
#endif
	real32 ScreenCenterX = 0.5f * (real32)Buffer->Width;
	real32 ScreenCenterY = 0.5f * (real32)Buffer->Height;

	entity_visible_piece_group PieceGroup;
	PieceGroup.GameState = GameState;
	sim_entity* Entity = SimRegion->Entities;
	
	for(uint32 EntityIndex = 0;
		EntityIndex < SimRegion->EntityCount;
		++EntityIndex, ++Entity)
	{
		if (Entity->Updatable)
		{
			PieceGroup.PieceCount = 0;

			real32 dt = Input->dtForFrame;

			real32 ShadowAlpha = 1.0f - 0.5f * Entity->P.Z;
			if (ShadowAlpha < 0)
			{
				ShadowAlpha = 0.0f;
			}

			move_spec MoveSpec = DefaultMoveSpec();
			v3 ddP = {};

			hero_bitmaps* HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
			switch (Entity->Type)
			{
			case EntityType_Hero:
			{
				for (uint32 ControllerIndex = 0;
					ControllerIndex < ArrayCount(GameState->ControlledHeros);
					++ControllerIndex
					)
				{
					controlled_hero* ConHero = GameState->ControlledHeros + ControllerIndex;
					if (Entity->StorageIndex == ConHero->EntityIndex)
					{
						if (ConHero->dZ != 0.0f)
						{
							Entity->dP.Z = ConHero->dZ;
						}
						MoveSpec.UnitMaxAccelVector = false;
						MoveSpec.Drag = 8.0f;
						MoveSpec.Speed = 40.0f;
						ddP = V3(ConHero->ddP,0);

						if ((ConHero->dSword.X != 0.0f) || (ConHero->dSword.Y != 0.0f))
						{
							sim_entity* Sword = Entity->Sword.Ptr;

							if (Sword && IsSet(Sword, EntityFlag_Nonspatial))
							{
								Sword->DistanceLimit = 5.0f;
								MakeEntitySpatial(Sword, Entity->P,
												 Entity->dP + 5.0f * V3(ConHero->dSword,0));
								AddCollisionRule(GameState, Sword->StorageIndex, Entity->StorageIndex, false);
							}
						}
					}
				}

				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
				PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0, 0), 0, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &HeroBitmaps->Cape, V2(0, 0), 0, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0, HeroBitmaps->Align);

				DrawHitPoints(Entity, &PieceGroup);
			}break;

			case EntityType_Wall:
			{
				PushBitmap(&PieceGroup, &GameState->Tree, V2(0, 0), 0, V2(40, 80));
			}break;

			case EntityType_Stairwell:
			{
				PushRect(&PieceGroup, V2(0, 0), 0, Entity->WalkableDim, V4(1, 0.5f, 0, 1), 0.0f);
				//PushRect(&PieceGroup, V2(0,0),Entity->WalkableHeight, Entity->WalkableDim, V4(1,1,0,1),0.0f);
			}break;

			case EntityType_Sword:
			{

				MoveSpec.UnitMaxAccelVector = false;
				MoveSpec.Drag = 0.0f;
				MoveSpec.Speed = 0.0f;

				if (Entity->DistanceLimit == 0.0f)
				{
					ClearCollisionRulesFor(GameState, Entity->StorageIndex);
					MakeEntityNonSpatial(Entity);
					
				}
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
				PushBitmap(&PieceGroup, &GameState->Sword, V2(0, 0), 0, V2(29, 10));

			}break;

			case EntityType_Familiar:
			{
				sim_entity* ClosestHero = 0;
				real32 ClosestHeroDSq = Square(10.0f);
				//Familiar doesnt find hero ever
#if 0
				sim_entity* TestEntity = SimRegion->Entities;
				for (uint32 TestEntityIndex = 0;
					TestEntityIndex < SimRegion->EntityCount;
					++TestEntityIndex, ++TestEntity)
				{

					if (TestEntity->Type == EntityType_Hero)
					{
						real32 TestDSq = LengthSq(TestEntity->P - Entity->P);
						if (ClosestHeroDSq > TestDSq)
						{
							ClosestHero = TestEntity;
							ClosestHeroDSq = TestDSq;
						}
					}
				}
#endif
				if (ClosestHero && (ClosestHeroDSq > Square(2.0f)))
				{
					real32 Acceleration = 0.03f; // trying to slow down originally 0.3
					real32 OneOverLength = (Acceleration / SquareRoot(ClosestHeroDSq));
					ddP = OneOverLength * (ClosestHero->P - Entity->P);
				}
				
				MoveSpec.UnitMaxAccelVector = true;
				MoveSpec.Drag = 8.0f;
				MoveSpec.Speed = 50.0f;

				Entity->tBob += dt;
				if (Entity->tBob > (2.0f * Pi32))
				{
					Entity->tBob -= (2.0f * Pi32);
				}
				real32 BobSin = Sin(4.0f * Entity->tBob);
				PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0.1f * BobSin, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, 0.2f * ShadowAlpha + 0.1f * BobSin, 0.0f);
			}break;

			case EntityType_Monster:
			{
				PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0, 0), 0, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
				DrawHitPoints(Entity, &PieceGroup);
			}break;

			case EntityType_Space:
			{
				for (uint32 VolumeIndex = 0;
					VolumeIndex < Entity->Collision->VolumeCount;
					VolumeIndex++)
				{
					sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
					PushRectOutline(&PieceGroup,Volume->OffsetP.XY, 0, Volume->Dim.XY, V4(0, 0.5f, 1.0f, 1), 0.0f);
				}

			}break;

			default:
			{
				InvalidCodePath;
			}break;
			}

			if (!IsSet(Entity, EntityFlag_Nonspatial) &&
				IsSet(Entity, EntityFlag_Moveable))
			{
				MoveEntity(GameState, SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP);
			}
			
			real32 ZFudge = (1.0f + 0.1 * Entity->P.Z);

			real32 EntityGroundPointX = ScreenCenterX + MetersToPixels * ZFudge *Entity->P.X;
			real32 EntityGroundPointY = ScreenCenterY - MetersToPixels * ZFudge *Entity->P.Y;
			real32 EntityZ = -MetersToPixels * Entity->P.Z;


#if 0		
			v2 PlayerLeftTop = { PlayerGroundPointX - 0.5f * MetersToPixels * LowEntity->Width,
								PlayerGroundPointY - 0.5f * MetersToPixels * LowEntity->Height };
			v2 EntityWidthHeight = { LowEntity->Width, LowEntity->Height };
#endif		
			for (uint32 PieceIndex = 0;
				PieceIndex < PieceGroup.PieceCount;
				++PieceIndex)
			{
				entity_visible_piece* Piece = PieceGroup.Pieces + PieceIndex;
				v2 Center = { EntityGroundPointX + Piece->Offset.X,
							  EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + Piece->EntityZC * EntityZ };
				if (Piece->Bitmap)
				{
					DrawBitmap(Buffer, Piece->Bitmap, Center.X, Center.Y, Piece->A);
				}
				else
				{
					v2 HalfDim = 0.5f * MetersToPixels * Piece->Dim;
					DrawRectangle(Buffer, Center - HalfDim, Center + HalfDim,
						Piece->R, Piece->G, Piece->B);
				}

			}
		}
	}

	world_position WorldOrigin = {};
	v3 Diff = Subtract(SimRegion->World, &WorldOrigin, &SimRegion->Origin);
	DrawRectangle(Buffer, Diff.XY, V2(10.0f, 10.0f), 1.0f, 1.0f, 0.0f);

	EndSim(SimRegion, GameState);
}//End of GameUpdateAndRenderer

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
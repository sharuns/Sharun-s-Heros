/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/
#include "SharunMade.h"
#include "SharunMadeWorld.cpp"
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

inline v2
GetCameraSpace(game_state* GameState, low_entity* EntityLow)
{
	world_difference Diff = Subtract(GameState->World, &EntityLow->P, &GameState->CameraP);
	v2 Result = Diff.dXY;

	return (Result);
}

inline high_entity *
MakeEntityHighFrequency(game_state* GameState, low_entity * EntityLow , uint32 LowIndex, v2 CameraSpaceP)
{
 	high_entity* EntityHigh = 0;

	Assert(EntityLow->HighEntityIndex == 0);
	if (EntityLow->HighEntityIndex == 0)
	{
		if (GameState->HighEntityCount < ArrayCount(GameState->HighEntities_))
		{
			uint32 HighIndex = GameState->HighEntityCount++;
			EntityHigh = GameState->HighEntities_ + HighIndex;

			EntityHigh->P = CameraSpaceP;
			EntityHigh->dP = v2{ 0, 0 };
			EntityHigh->ChunkZ = EntityLow->P.ChunkZ;
			EntityHigh->FacingDirection = 0;
			EntityHigh->LowEnitityIndex = LowIndex;

			EntityLow->HighEntityIndex = HighIndex;

		}
		else
		{
			InvalidCodePath;
		}
	}
	return (EntityHigh);
}

inline high_entity*
MakeEntityHighFrequency(game_state* GameState, uint32 LowIndex)
{
	high_entity* EntityHigh = 0;

	low_entity* EntityLow = GameState->LowEntities + LowIndex;
	if (EntityLow->HighEntityIndex)
	{
		EntityHigh = GameState->HighEntities_ + EntityLow->HighEntityIndex;
	}
	else
	{
		v2 CameraSpaceP = GetCameraSpace(GameState, EntityLow);
		EntityHigh = MakeEntityHighFrequency(GameState, EntityLow, LowIndex, CameraSpaceP);
	}
	
	return(EntityHigh);
}


inline void
MakeEntityLowFrequency(game_state* GameState, uint32  LowIndex)
{
	low_entity* EntityLow = &GameState->LowEntities[LowIndex];
	uint32 HighIndex = EntityLow->HighEntityIndex;

	if (HighIndex)
	{
			uint32 LastHighIndex = GameState->HighEntityCount -1;
			if (HighIndex != LastHighIndex)
			{
				high_entity* LastEntity = GameState->HighEntities_ + LastHighIndex;
				high_entity* DelEntity = GameState->HighEntities_ + HighIndex;

				*DelEntity = *LastEntity;
				GameState->LowEntities[LastEntity->LowEnitityIndex].HighEntityIndex = HighIndex;
			}
			--GameState->HighEntityCount;
			EntityLow->HighEntityIndex = 0; 
	}
}


inline low_entity *
GetLowEntity(game_state * GameState, uint32 Index)
{
	low_entity *Result = 0;
	if((Index > 0 ) && (Index < GameState->LowEntityCount))
	{
		Result = GameState->LowEntities + Index;
	}
	return (Result);
}


inline entity
ForceEntityToHigh(game_state* GameState, uint32 LowIndex)
{
	entity Result = {};
	if ((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
	{
		Result.LowIndex = LowIndex;
		Result.Low = GameState->LowEntities + LowIndex;
		Result.High = MakeEntityHighFrequency(GameState, LowIndex);
	}
	return (Result);
}


inline void
OffsetAndCheckFrequencyByArea(game_state * GameState, v2 Offset, rectangle2 HighFrequencyBounds)
{
	for (uint32 HighEntityIndex = 1;
		HighEntityIndex < GameState->HighEntityCount;
		)
	{

		high_entity* High = GameState->HighEntities_ + HighEntityIndex;
		low_entity* Low = GameState->LowEntities + High->LowEnitityIndex;
		High->P += Offset;

		if (IsValid(Low->P) && IsInRectangle(HighFrequencyBounds, High->P))
		{
			++HighEntityIndex;
		}
		else
		{
			Assert(GameState->LowEntities[High->LowEnitityIndex].HighEntityIndex == HighEntityIndex);
			MakeEntityLowFrequency(GameState, High->LowEnitityIndex);
		}
	}
}

struct add_low_entity_result
{
	low_entity* Low;
	uint32 LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state* GameState, entity_type Type, world_position *P)
{
	Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
	uint32 EntityIndex = GameState->LowEntityCount++;

	low_entity* EntityLow = GameState->LowEntities + EntityIndex;
	*EntityLow = {};
	EntityLow->Type = Type;

	
	ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, 0, P);
	
	add_low_entity_result Result;
	Result.Low = EntityLow;
	Result.LowIndex = EntityIndex;

	return (Result);
}



internal add_low_entity_result
AddWall(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall,&P);

	Entity.Low->P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	Entity.Low->Height = GameState->World->TileSideInMeters;
	Entity.Low->Width = Entity.Low->Height;
	Entity.Low->Collides = true;

	return (Entity);
}

internal void
InitHitPoints(low_entity * EntityLow, uint32 HitPointCount)
{
	Assert(HitPointCount <= ArrayCount(EntityLow->HitPoint));
	EntityLow->HitPointMax = HitPointCount;
	for (uint32 HitPointIndex = 0;
		HitPointIndex < EntityLow->HitPointMax;
		++HitPointIndex)
	{
		hit_point* HitPoint = EntityLow->HitPoint + HitPointIndex;
		HitPoint->Flags = 0;
		HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
	}
}

internal add_low_entity_result
AddSword(game_state* GameState)
{
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, 0);

	Entity.Low->Height = 0.5f;
	Entity.Low->Width = 1.0f;
	Entity.Low->Collides = false;

	return (Entity);
}

internal add_low_entity_result
AddPlayer( game_state * GameState)
{
	world_position P = GameState->CameraP;;
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, &P);
	Entity.Low->Height = 0.5f;
	Entity.Low->Width = 1.0f;
	Entity.Low->Collides = true;

	InitHitPoints(Entity.Low, 3);

	add_low_entity_result Sword = AddSword(GameState);
	Entity.Low->SwordLowIndex = Sword.LowIndex;

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
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monster, &P);

	Entity.Low->Height = 0.5f;
	Entity.Low->Width = 1.0f;
	Entity.Low->Collides = true;

	InitHitPoints(Entity.Low, 3);

	return (Entity);
}

internal add_low_entity_result
AddFamiliar(game_state* GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Familiar, &P);

	Entity.Low->Height = 0.5f;
	Entity.Low->Width = 1.0f;
	Entity.Low->Collides = false;

	return (Entity);
}


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

struct move_spec
{
	bool32 UnitMaxAccelVector;
	real32 Speed;
	real32 Drag;
};

inline move_spec
DefaultMoveSpec(void)
{
	move_spec Result;
	
	Result.UnitMaxAccelVector = false;
	Result.Drag = 0.0f;
	Result.Speed = 1.0f;

	return (Result);
}

internal void
MoveEntity(game_state* GameState, entity Entity, move_spec * MoveSpec,real32 dt, v2 ddP) 
{
	world* World = GameState->World;

	if (MoveSpec->UnitMaxAccelVector)
	{
		real32 ddPLength = LengthSq(ddP);
		if (ddPLength > 1.0f) {

			ddP *= (1.0f / SquareRoot(ddPLength));
		}
	}

	//This is default acceleration value of the player

	ddP *= MoveSpec->Speed;

	ddP += -MoveSpec->Drag * Entity.High->dP;

	v2 OldPlayerP = Entity.High->P;
	v2 PlayerDelta = (0.5f * ddP * Square(dt) +
						Entity.High->dP * dt);
	// velocity equation v' = (at + v)
	Entity.High->dP = ddP * dt + Entity.High->dP;
	v2 NewPlayerP = OldPlayerP + PlayerDelta;

	//position equation p' = (1/2 * at^2 + Vt + p)
	/*collision detection : based on the Basic Minkowski collision detection*/

	for (uint32 Iterations = 0;
		Iterations < 4;
		++Iterations)
	{
		real32 tMin = 1.0f;
		v2 WallNormal = {};
		uint32 HitHighEntityIndex = 0; 
		v2 DesiredPosition = Entity.High->P + PlayerDelta;
		if (Entity.Low->Collides)
		{
			for (uint32 TestHighEntityIndex = 1;
				TestHighEntityIndex < GameState->HighEntityCount;
				++TestHighEntityIndex)
			{
				if (TestHighEntityIndex != Entity.Low->HighEntityIndex)
				{
					entity TestEntity;
					TestEntity.High = GameState->HighEntities_ + TestHighEntityIndex;;
					TestEntity.LowIndex = TestEntity.High->LowEnitityIndex;
					TestEntity.Low = GameState->LowEntities + TestEntity.LowIndex;
					if (TestEntity.Low->Collides)
					{
						real32 DiameterW = TestEntity.Low->Width + Entity.Low->Width;
						real32 DiameterH = TestEntity.Low->Height + Entity.Low->Height;

						v2 MinCorner = -0.5f * v2{ DiameterW, DiameterH };
						v2 MaxCorner = 0.5f * v2{ DiameterW, DiameterH };

						v2 Rel = Entity.High->P - TestEntity.High->P;

						if (TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
							&tMin, MinCorner.Y, MaxCorner.Y))
						{

							WallNormal = v2{ -1,0 };
							HitHighEntityIndex = TestHighEntityIndex;
						}

						if (TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
							&tMin, MinCorner.Y, MaxCorner.Y))
						{
							WallNormal = v2{ 1,0 };
							HitHighEntityIndex = TestHighEntityIndex;
						}

						if (TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
							&tMin, MinCorner.X, MaxCorner.X))
						{
							WallNormal = v2{ 0,-1 };
							HitHighEntityIndex = TestHighEntityIndex;
						}

						if (TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
							&tMin, MinCorner.X, MaxCorner.X))
						{
							WallNormal = v2{ 0,1 };
							HitHighEntityIndex = TestHighEntityIndex;
						}

					}
				}
			}
		}
		//end of collision dectection

		Entity.High->P += tMin * PlayerDelta;
		
		if (HitHighEntityIndex)
		{
			Entity.High->dP = Entity.High->dP - 1 * Inner(Entity.High->dP, WallNormal) * WallNormal;
			PlayerDelta = DesiredPosition - Entity.High->P;
			PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;

			high_entity* HitHigh = GameState->HighEntities_ + HitHighEntityIndex;
			low_entity* HitLow = GameState->LowEntities + HitHigh->LowEnitityIndex;
			//Entity.High->AbsTileZ += HitLow->dAbsTileZ;
		}
		else
		{
			break;
		}
	
	}
	//position equation p' = (1/2 * at^2 + Vt + p)

	if(Entity.High->dP.X == 0.0f && (Entity.High->dP.Y == 0.0f)){

	}
	else if(AbsoluteValue(Entity.High->dP.X) > AbsoluteValue(Entity.High->dP.Y))
	{

		if(Entity.High->dP.X > 0)
		{	
			Entity.High->FacingDirection = 0; //[ssk] : Right facing
		}
		else
		{
			Entity.High->FacingDirection = 2; //[ssk] : Left facing
		}

	}
	else
	{
	
		if(Entity.High->dP.Y > 0)
		{	
			Entity.High->FacingDirection = 1; //[ssk] : Back facing
		}
		else
		{
			Entity.High->FacingDirection = 3; ////[ssk] : Front facing
		}

	}
	world_position NewP = MapIntoChunkSpace(GameState->World, GameState->CameraP, Entity.High->P);
	ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex,Entity.Low, &Entity.Low->P, &NewP);
	//Entity.Low->P = NewP;

}

/*
Set camera basically keeps the entities that are in camera space into HIGH entity and moves other to Low entity space 
And vice-versa as well.
*/
internal void
SetCamera(game_state *GameState, world_position NewCameraP)
{
	world* World = GameState->World;

	world_difference dCameraP = Subtract(World, &NewCameraP , &GameState->CameraP);
	GameState->CameraP = NewCameraP;

	uint32 TileSpanX = 17 * 3;
	uint32 TileSpanY = 9 * 3;
	rectangle2 CameraBounds = RectCenterDim(V2(0, 0), 
											World->TileSideInMeters * V2((real32)TileSpanX,
																		   (real32)TileSpanY));

	v2 EntityOffsetForFrame = -dCameraP.dXY;
	OffsetAndCheckFrequencyByArea(GameState,EntityOffsetForFrame, CameraBounds);


	world_position MinChunkP = MapIntoChunkSpace(World, NewCameraP, GetMinCorner(CameraBounds));
	world_position MaxChunkP = MapIntoChunkSpace(World, NewCameraP, GetMaxCorner(CameraBounds));

	for (int32 ChunkY = MinChunkP.ChunkY;
		ChunkY <= MaxChunkP.ChunkY;
		++ChunkY)
	{
		for (int32 ChunkX = MinChunkP.ChunkX;
			ChunkX <= MaxChunkP.ChunkX;
			++ChunkX)
		{
			world_chunk* Chunk = GetWorldChunk(World, ChunkX, ChunkY, NewCameraP.ChunkZ);
			if (Chunk)
			{
				for(world_entity_block* Block = &Chunk->FirstBlock;
					Block;
					Block = Block->Next)
				{
					for (uint32 EntityIndexIndex = 0;
						EntityIndexIndex < Block->EntityCount;
						++EntityIndexIndex)
					{
						uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
						low_entity* Low = GameState->LowEntities + LowEntityIndex;
						if (Low->HighEntityIndex == 0)
						{
							v2 CameraSpaceP = GetCameraSpace(GameState, Low);
							if (IsInRectangle(CameraBounds, CameraSpaceP))
							{
								MakeEntityHighFrequency(GameState, Low , LowEntityIndex, CameraSpaceP);
							}
						}
					}
				}
			}
		}
	}
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
DrawHitPoints(low_entity * LowEntity, entity_visible_piece_group * PieceGroup)
{
	if (LowEntity->HitPointMax >= 1)
	{
		v2 HealthDim = { 0.2f,0.2f };
		real32 SpacingX = 1.5f * HealthDim.X;
		v2 HitP = { -0.5f * (LowEntity->HitPointMax - 1) * SpacingX, -0.25f };
		v2 dHitP = { SpacingX, 0.0f };
		for (uint32 HealthIndex = 0;
			HealthIndex < LowEntity->HitPointMax;
			++HealthIndex)
		{
			hit_point* HitPoint = LowEntity->HitPoint + HealthIndex;
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

inline entity
EntityFromHighIndex(game_state* GameState, uint32 HighEntityIndex)
{
	entity Result = {};

	if (HighEntityIndex)
	{
		Assert(HighEntityIndex < ArrayCount(GameState->HighEntities_));
		Result.High = GameState->HighEntities_ + HighEntityIndex;
		Result.LowIndex = Result.High->LowEnitityIndex;
		Result.Low = GameState->LowEntities + Result.LowIndex;
	}

	return (Result);
}

inline void
UpdateMonster(game_state* GameState, entity Entity, real32 dt)
{
}

inline void
UpdateSword(game_state* GameState, entity Entity, real32 dt)
{
	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.UnitMaxAccelVector = false;
	MoveSpec.Drag = 0.0f;
	MoveSpec.Speed = 0.0f;

	v2 OldP = Entity.High->P;
	MoveEntity(GameState, Entity, &MoveSpec, dt, V2(0, 0));
	real32 DistanceTraveled = Length(Entity.High->P - OldP);

	Entity.Low->DistanceRemaining -= DistanceTraveled;
	if (Entity.Low->DistanceRemaining < 0)
	{
		ChangeEntityLocation(&GameState->WorldArena, GameState->World, 
							Entity.LowIndex, Entity.Low, &Entity.Low->P, 0);
		//Entity.High->P += V2(1000000.0f, 1000000.0f);
	}
}

inline void
UpdateFamiliar(game_state* GameState, entity Entity, real32 dt)
{
	entity ClosestHero = {};
	real32 ClosestHeroDSq = Square(10.0f);
	for (uint32 HighEntityIndex = 1;
		HighEntityIndex < GameState->HighEntityCount;
		++HighEntityIndex)
	{
		entity TestEntity = EntityFromHighIndex(GameState, HighEntityIndex);

		if (TestEntity.Low->Type == EntityType_Hero)
		{
			real32 TestDSq = LengthSq(TestEntity.High->P - Entity.High->P);
			if (TestEntity.Low->Type == EntityType_Hero)
			{
				TestDSq *= 0.75f;
			}
			if(ClosestHeroDSq > TestDSq)
			{
				ClosestHero = TestEntity;
				ClosestHeroDSq = TestDSq;
			}
		}
	}

	v2 ddP = {};
	if (ClosestHero.High && (ClosestHeroDSq > Square(2.0f)))
	{
		real32 Acceleration = 0.5f;
		real32 OneOverLength = (Acceleration / SquareRoot(ClosestHeroDSq));
		ddP = OneOverLength*(ClosestHero.High->P - Entity.High->P);
	}
	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.UnitMaxAccelVector = true;
	MoveSpec.Drag = 8.0f;
	MoveSpec.Speed = 50.0f;
	MoveEntity(GameState, Entity, &MoveSpec, dt, ddP);
}

extern "C" GAME_UPDATE_AND_RENDERER(GameUpdateAndRenderer)
{
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state * GameState = (game_state * )Memory->PermanentStorage;

	if(!Memory->IsInitialized){

		//So that the 0th index is not used
		
		AddLowEntity(GameState, Entity_Null, 0);
		GameState->HighEntityCount = 1;

		GameState->Backdrop = DEBUGLoadBMP(Thread ,Memory->DEBUGPlatformReadEntireFile, "data/test/test_background.bmp");
		GameState->Shadow = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test/test_hero_shadow.bmp");
		GameState->Tree = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test2/tree00.bmp");
		GameState->Sword = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "data/test2/rock03.bmp");
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
		
		InitailizeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
						(uint8 * )Memory->PermanentStorage + sizeof(game_state));


		GameState->World = PushStruct(&GameState-> WorldArena, world);
		world* World = GameState->World;
		InitializeWorld(World,1.4f);//1.4f is the tiles in meters size
		
		int32 TilesSideInPixels = 60;
		GameState->MetersToPixels = (real32)TilesSideInPixels / (real32)World->TileSideInMeters;
		
		uint32 RandomNumberIndex = 0 ;
		
		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;
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
			
			//if(DoorUp ||DoorDown)
			{
			RandomChoice = RandomNumberTable[RandomNumberIndex++]%2;
			}
#if 0
			else{
				RandomChoice = RandomNumberTable[RandomNumberIndex++]%3;
			}
#endif
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
					uint32 TileValue = 1;

					if(( TileX == 0 ) && (!DoorLeft || ( TileY != (TilesPerHeight/2))))
					{
						TileValue =2;
					}
					if(( TileX == (TilesPerWidth -1) ) && (!DoorRight|| ( TileY != (TilesPerHeight/2))))
					{
						TileValue =2;
					}
					if(( TileY == 0 ) && (!DoorBottom || (TileX != (TilesPerWidth/2))))
					{	
						TileValue = 2;
					}
					if(( TileY == (TilesPerHeight -1) ) && (!DoorTop || (TileX != (TilesPerWidth/2))))
					{	
						TileValue = 2;
					}
					if((TileX == 10)&&(TileY == 6))
					{
						if(DoorUp)
						{
							TileValue = 3;
						}
						if(DoorDown)
						{
							TileValue = 4;
						}

					}
					//[ssk] : Add wall entity based on the tile value 52 wall tiles ??
					if (TileValue == 2 )
					{
						AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
					}
					
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
		
		AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);

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

		SetCamera(GameState, NewCameraP);

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
		uint32 LowIndex = GameState->PlayerIndexForController[ControllerIndex];
		if (LowIndex == 0)
		{
			//[ssk] : On hiting space we add a player entity
			if (Controller->Start.EndedDown)
			{
				uint32 EntityIndex = AddPlayer(GameState).LowIndex;
				GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
			}
		}
		else
		{
			entity ControllingEntity = ForceEntityToHigh(GameState, LowIndex);

			v2 ddP = {};

			if (Controller->IsAnalog)
			{
				ddP = v2{ Controller->StickAverageX, Controller->StickAverageY };
			}
			else
			{
				//digital controller = Key
				if (Controller->MoveUp.EndedDown)
				{
					ddP.Y = 1.0f;
				}
				if (Controller->MoveDown.EndedDown) 
				{
					ddP.Y = -1.0f;
				}
				if (Controller->MoveLeft.EndedDown) 
				{
					ddP.X = -1.0f;
				}
				if (Controller->MoveRight.EndedDown) 
				{
					ddP.X = 1.0f;
				}
			}
			if (Controller->Start.EndedDown)
			{
				ControllingEntity.High->dZ = 3.0f;
			}

			v2 dSword = {};

			if (Controller->ActionUp.EndedDown)
			{
				dSword = V2(0.0f, 1.0f);
			}
			if (Controller->ActionDown.EndedDown)
			{
				dSword = V2(0.0f, -1.0f);
			}
			if (Controller->ActionLeft.EndedDown)
			{
				dSword = V2(-1.0f, 0.0f);
			}
			if (Controller->ActionRight.EndedDown)
			{
				dSword = V2(1.0f, 0.0f);
			}

			move_spec MoveSpec = DefaultMoveSpec();
			MoveSpec.UnitMaxAccelVector = false;
			MoveSpec.Drag = 8.0f;
			MoveSpec.Speed = 50.0f;
			MoveEntity(GameState, ControllingEntity, &MoveSpec, Input->dtForFrame, ddP);
			if ((dSword.X != 0.0f) || (dSword.Y != 0.0f)) 
			{
				low_entity* LowSword = GetLowEntity(GameState, ControllingEntity.Low->SwordLowIndex);
				if (LowSword && !IsValid(LowSword->P))
				{
					world_position SwordP = ControllingEntity.Low->P;
					ChangeEntityLocation(&GameState->WorldArena, GameState->World, 
										ControllingEntity.Low->SwordLowIndex,LowSword, 0, &SwordP);

					entity Sword = ForceEntityToHigh(GameState, ControllingEntity.Low->SwordLowIndex);
					Sword.Low->DistanceRemaining = 5.0f; 
					Sword.High->dP = 2.0f*dSword;
				}
			}
		}
	}//[ssk] : End of for loop for controller

	//[ssk] : Camera control and movement 
	
	entity CameraFollowingEntity = ForceEntityToHigh(GameState, GameState->CameraFollowingEntityIndex);

	if(CameraFollowingEntity.High)
	{

		world_position NewCameraP = GameState->CameraP;
	
		NewCameraP.ChunkZ = CameraFollowingEntity.Low->P.ChunkZ;
#if 0
		if(CameraFollowingEntity.High->P.X  >(9.0f*World->TileSideInMeters))
		{
			NewCameraP.ChunkX += 17;
		}
		if(CameraFollowingEntity.High->P.X < -(9.0f*World->TileSideInMeters))
		{
			NewCameraP.ChunkX -= 17;
		}
		if(CameraFollowingEntity.High->P.Y >(5.0f*World->TileSideInMeters))
		{
			NewCameraP.ChunkY += 9;
		}
		if(CameraFollowingEntity.High->P.Y < -(5.0f*World->TileSideInMeters))
		{
			NewCameraP.ChunkY -= 9;
		}
#else
		NewCameraP = CameraFollowingEntity.Low->P;
#endif
		 SetCamera(GameState, NewCameraP);
	}
	
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
	
	for(uint32 HighEntityIndex = 1;
		HighEntityIndex < GameState->HighEntityCount;
		++HighEntityIndex)
	{
		PieceGroup.PieceCount = 0;
		high_entity* HighEntity = GameState->HighEntities_ + HighEntityIndex;
		low_entity* LowEntity = GameState->LowEntities + HighEntity->LowEnitityIndex;


		entity Entity;
		Entity.LowIndex = HighEntity->LowEnitityIndex;
		Entity.Low = LowEntity;
		Entity.High = HighEntity;
		//UpdateEntity(GameState, Entity, dt);

		real32 dt = Input->dtForFrame;
		
		real32 ShadowAlpha = 1.0f - 0.5f * HighEntity->Z;
		if (ShadowAlpha < 0)
		{
			ShadowAlpha = 0.0f;
		}

		hero_bitmaps* HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];
		switch (LowEntity->Type)
		{
			case EntityType_Hero:
			{
				PushBitmap(&PieceGroup, &GameState->Shadow,  V2(0, 0),0, HeroBitmaps->Align, ShadowAlpha,0.0f);
				PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0, 0), 0, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &HeroBitmaps->Cape,  V2(0, 0), 0, HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &HeroBitmaps->Head,  V2(0, 0), 0, HeroBitmaps->Align);

				DrawHitPoints(LowEntity, &PieceGroup);
			}break;

			case EntityType_Wall:
			{
				PushBitmap(&PieceGroup, &GameState->Tree, V2(0,0),0, V2(40, 80));
			}break;

			case EntityType_Sword:
			{
				UpdateSword(GameState, Entity, dt);
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
				PushBitmap(&PieceGroup, &GameState->Sword, V2(0, 0), 0, V2(29, 10));

			}break;

			case EntityType_Familiar:
			{
				UpdateFamiliar(GameState, Entity, dt);
				Entity.High->tBob += dt;
				if (Entity.High->tBob > (2.0f * Pi32))
				{
					Entity.High->tBob -= (2.0f * Pi32);
				}
				real32 BobSin = Sin(4.0f * Entity.High->tBob);
				PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0,0), 0.1f*BobSin , HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0,0), 0, HeroBitmaps->Align,0.2f*ShadowAlpha + 0.1f * BobSin,0.0f);
			}break;

			case EntityType_Monster:
			{
				UpdateMonster(GameState, Entity, dt);
				PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0,0), 0,HeroBitmaps->Align);
				PushBitmap(&PieceGroup, &GameState->Shadow, V2(0,0), 0,HeroBitmaps->Align,ShadowAlpha, 0.0f);
				DrawHitPoints(LowEntity, &PieceGroup);
			}break;

			default:
			{
				InvalidCodePath;
			}break;
		}

		real32 ddZ = -9.8f;
		HighEntity->Z = 0.5f * ddZ * Square(dt) + HighEntity->dZ * dt + HighEntity->Z;
		HighEntity->dZ = ddZ * dt + HighEntity->dZ;

		if (HighEntity->Z < 0)
		{
			HighEntity->Z = 0;
		}

		real32 EntityGroundPointX = ScreenCenterX + MetersToPixels * HighEntity->P.X;
		real32 EntityGroundPointY = ScreenCenterY - MetersToPixels * HighEntity->P.Y;
		real32 EntityZ = -MetersToPixels * HighEntity->Z;


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
						  EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + Piece->EntityZC * EntityZ};
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
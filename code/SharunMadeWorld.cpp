/*
Date : 26th Nov 2021
Author : Sharun S
Place : Chennai , India
*/

#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define TILE_CHUNK_UNINTITALIZED INT32_MAX
#define TILES_PER_CHUNK 16


inline world_position
NullPosition(void)
{
	world_position Result = {};

	Result.ChunkX = TILE_CHUNK_UNINTITALIZED;

	return(Result);
}

inline bool32 
IsValid(world_position P)
{
	bool32 Result = (P.ChunkX != TILE_CHUNK_UNINTITALIZED);
	return (Result);
}

inline bool32
IsCannonical(real32 ChunkDim, real32 TileRel)
{
	real32 Epsilon = 0.01f;
	bool32 Result = ((TileRel >= -(0.5f * ChunkDim + Epsilon)) &&
					(TileRel <= (0.5f * ChunkDim + Epsilon)));
	return (Result);
}

inline bool32
IsCannonical(world* World, v3 Offset)
{
	bool32 Result = (IsCannonical(World->ChunkDimInMeters.X, Offset.X) &&
					 IsCannonical(World->ChunkDimInMeters.Y, Offset.Y) &&
					 IsCannonical(World->ChunkDimInMeters.Z, Offset.Z));
	return (Result);
}

inline bool32
AreOnSameChunk(world * World, world_position* A, world_position* B) 
{
	Assert(IsCannonical(World, A->Offset_));
	Assert(IsCannonical(World, B->Offset_));

	bool32 Result = ((A->ChunkX == B->ChunkX) &&
					 (A->ChunkY == B->ChunkY) &&
					 (A->ChunkZ == B->ChunkZ));
	return (Result);

}


inline world_chunk*
GetWorldChunk(world* World, int32 ChunkX, int32 ChunkY, int32 ChunkZ,
	memory_arena* Arena = 0)
{
	Assert(ChunkX > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkY > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkZ > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkX < (TILE_CHUNK_SAFE_MARGIN));
	Assert(ChunkY < (TILE_CHUNK_SAFE_MARGIN));
	Assert(ChunkZ < (TILE_CHUNK_SAFE_MARGIN));

	//TODO : Basic Hashing : Can have a better hashing 
	uint32 HashValue = 19 * ChunkX + 7 * ChunkY + 3 * ChunkZ;
	uint32  HashSlot = HashValue & (ArrayCount(World->ChunkHash) - 1);
	Assert(HashSlot < ArrayCount(World->ChunkHash));

	world_chunk* Chunk = World->ChunkHash + HashSlot;
	do
	{
		if ((ChunkX == Chunk->ChunkX) &&
			(ChunkY == Chunk->ChunkY) &&
			(ChunkZ == Chunk->ChunkZ))
		{
			break;
		}
		if (Arena && (Chunk->ChunkX != TILE_CHUNK_UNINTITALIZED) && (!Chunk->NextInHash))
		{
			Chunk->NextInHash = PushStruct(Arena, world_chunk);
			Chunk = Chunk->NextInHash;
			Chunk->ChunkX = TILE_CHUNK_UNINTITALIZED;
		}

		if (Arena && (Chunk->ChunkX == TILE_CHUNK_UNINTITALIZED))
		{
			Chunk->ChunkX = ChunkX;
			Chunk->ChunkY = ChunkY;
			Chunk->ChunkZ = ChunkZ;

			Chunk->NextInHash = 0;
			break;
		}
		Chunk = Chunk->NextInHash;
	} while (Chunk);

	return (Chunk);
}

internal void
InitializeWorld(world* World, real32 TileSideInMeters)
{
	World->TileSideInMeters = TileSideInMeters;
	World->ChunkDimInMeters = {(real32)TILES_PER_CHUNK * TileSideInMeters,
								(real32)TILES_PER_CHUNK* TileSideInMeters,
								TileSideInMeters };
	World->TileDepthInMeters = (real32)TileSideInMeters;
	World->FirstFree = 0;

	for (uint32 ChunkIndex = 0;
		ChunkIndex < ArrayCount(World->ChunkHash);
		++ChunkIndex)
	{
		World->ChunkHash[ChunkIndex].ChunkX = TILE_CHUNK_UNINTITALIZED;
		World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
	}
}


inline void
ReCannonicalizeCoord(real32 ChunkDim, int32* Tile, real32* TileRel) {

	int32 Offset = RoundReal32ToInt32(*TileRel / ChunkDim);
	*Tile += Offset;
	*TileRel -= Offset * ChunkDim;

	Assert(IsCannonical(ChunkDim, *TileRel));
}


internal bool32
IsTileValueEmpty(uint32 TileValue)
{
	bool32 Empty = ((TileValue == 1) ||
		(TileValue == 3) ||
		(TileValue == 4));
	return (Empty);
}



inline world_position
MapIntoChunkSpace(world* World, world_position BasePos, v3 Offset) {

	world_position Result = BasePos;

	Result.Offset_ += Offset;
	ReCannonicalizeCoord(World->ChunkDimInMeters.X, &Result.ChunkX, &Result.Offset_.X);
	ReCannonicalizeCoord(World->ChunkDimInMeters.Y, &Result.ChunkY, &Result.Offset_.Y);
	ReCannonicalizeCoord(World->ChunkDimInMeters.Z, &Result.ChunkZ, &Result.Offset_.Z);

	return (Result);
}

inline world_position
ChunkPositionFromTilePosition(world * World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ, v3 AdditionalOffset = V3(0.0f, 0.0f, 0.0f))
{
	world_position BasePos = {};
	
	v3 Offset = World->TileSideInMeters * V3((real32)AbsTileX,(real32)AbsTileY,(real32)AbsTileZ);

	world_position Result = MapIntoChunkSpace(World, BasePos, AdditionalOffset + Offset);

	Assert(IsCannonical(World, Result.Offset_));

	return (Result);

}

inline v3
Subtract(world* World, world_position* A, world_position* B) 
{
	v3 dTile = { (real32)A->ChunkX - (real32)B->ChunkX,
				 (real32)A->ChunkY - (real32)B->ChunkY,
				 (real32)A->ChunkZ - (real32)B->ChunkZ };

	v3 Result = Hadamard(World->ChunkDimInMeters, dTile) + (A->Offset_ - B->Offset_);
	return (Result);

}

inline world_position
CenteredChunkPoint(uint32 ChunkX, uint32 ChunkY, uint32 ChunkZ) {

	world_position Result = {};

	Result.ChunkX = ChunkX;
	Result.ChunkY = ChunkY;
	Result.ChunkZ = ChunkZ;

	return (Result);
}


inline void 
ChangeEntityLocationRaw(memory_arena *Arena, world* World, uint32 LowEntityIndex,
	world_position* OldP, world_position* NewP)
{

	Assert(!OldP || IsValid(*OldP));
	Assert(!NewP || IsValid(*NewP));

	if (OldP && NewP && AreOnSameChunk(World, OldP, NewP))
	{
	}
	else
	{
		if (OldP)
		{
			world_chunk* Chunk = GetWorldChunk(World, OldP->ChunkX, OldP->ChunkY, OldP->ChunkZ, Arena);
			Assert(Chunk);
			if (Chunk)
			{
				bool32 NotFound = true;
				world_entity_block* FirstBlock = &Chunk->FirstBlock;
				for (world_entity_block* Block = &Chunk->FirstBlock;
					Block && NotFound;
					Block = Block->Next)
				{
					for (uint32 Index = 0;
						(Index < Block->EntityCount) && NotFound;
						++Index)
					{
						if (Block->LowEntityIndex[Index] == LowEntityIndex)
						{
							Block->LowEntityIndex[Index] =
								FirstBlock->LowEntityIndex[--FirstBlock->EntityCount];
							if (FirstBlock->EntityCount == 0)
							{
								if (FirstBlock->Next)
								{
									world_entity_block* NextBlock = FirstBlock->Next;
									*FirstBlock = *FirstBlock->Next;
									
									NextBlock->Next = World->FirstFree;
									World->FirstFree = NextBlock;
								}
							}

							NotFound = false;
						}
					}
				}
			}
		}

		if (NewP)
		{
			world_chunk* Chunk = GetWorldChunk(World, NewP->ChunkX, NewP->ChunkY, NewP->ChunkZ, Arena);
			world_entity_block* Block = &Chunk->FirstBlock;
			if (Block->EntityCount == ArrayCount(Block->LowEntityIndex))
			{
				world_entity_block* OldBlock = World->FirstFree;
				if (OldBlock)
				{
					World->FirstFree = OldBlock->Next;
				}
				else
				{
					OldBlock = PushStruct(Arena, world_entity_block);
				}

				*OldBlock = *Block;
				Block->Next = OldBlock;
				Block->EntityCount = 0;
			}

			Assert(Block->EntityCount < ArrayCount(Block->LowEntityIndex));
			Block->LowEntityIndex[Block->EntityCount++] = LowEntityIndex;
		}
	}
}


internal void 
ChangeEntityLocation(memory_arena* Arena, world* World, uint32 LowEntityIndex,
					low_entity* LowEntity, world_position NewPInit)
{
	world_position *OldP = 0;
	world_position* NewP = 0;

	if (!IsSet(&LowEntity->Sim, EntityFlag_Nonspatial) && IsValid(LowEntity->P))
	{
		OldP = &LowEntity->P;
	}

	if (IsValid(NewPInit))
	{
		NewP = &NewPInit;
	}

	ChangeEntityLocationRaw(Arena, World, LowEntityIndex, OldP, NewP);
	if (NewP)
	{
		LowEntity->P = *NewP;
		ClearFlags(&LowEntity->Sim, EntityFlag_Nonspatial);
	}
	else
	{
		LowEntity->P = NullPosition();
		AddFlags(&LowEntity->Sim, EntityFlag_Nonspatial);
	}
}
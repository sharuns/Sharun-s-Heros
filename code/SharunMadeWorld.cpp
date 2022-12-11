/*
Date : 26th Nov 2021
Author : Sharun S
Place : Chennai , India
*/

#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define TILE_CHUNK_UNINTITALIZED INT32_MAX
#define TILES_PER_CHUNK 16

inline bool32
IsCannonical(world* World, real32 TileRel)
{
	bool32 Result = ((TileRel >= -0.5f * World->ChunkSideInMeters) &&
		(TileRel <= 0.5f * World->ChunkSideInMeters));
	return (Result);
}

inline bool32
IsCannonical(world* World, v2 Offset)
{
	bool32 Result = (IsCannonical(World, Offset.X) && IsCannonical(World, Offset.Y));
	return (Result);
}

inline bool32
AreOnSameChunk(world * World, world_position* A, world_position* B) {

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
	World->ChunkSideInMeters = 16.0f * TileSideInMeters;

	for (uint32 ChunkIndex = 0;
		ChunkIndex < ArrayCount(World->ChunkHash);
		++ChunkIndex)
	{
		World->ChunkHash[ChunkIndex].ChunkX = TILE_CHUNK_UNINTITALIZED;
		World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
	}
}


inline void
ReCannonicalizeCoord(world* World, int32* Tile, real32* TileRel) {

	int32 Offset = RoundReal32ToInt32(*TileRel / World->ChunkSideInMeters);
	*Tile += Offset;
	*TileRel -= Offset * World->ChunkSideInMeters;

	Assert(IsCannonical(World, *TileRel));
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
MapIntoChunkSpace(world* World, world_position BasePos, v2 Offset) {

	world_position Result = BasePos;

	Result.Offset_ += Offset;
	ReCannonicalizeCoord(World, &Result.ChunkX, &Result.Offset_.X);
	ReCannonicalizeCoord(World, &Result.ChunkY, &Result.Offset_.Y);

	return (Result);
}

inline world_position
ChunkPositionFromTilePosition(world * World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
	world_position Result = {};
	Result.ChunkX = AbsTileX / TILES_PER_CHUNK;
	Result.ChunkY = AbsTileY / TILES_PER_CHUNK;
	Result.ChunkZ = AbsTileZ / TILES_PER_CHUNK;

	if (AbsTileX < 0)
	{
		--Result.ChunkX;
	}
	if (AbsTileY < 0)
	{
		--Result.ChunkY;
	}
	if (AbsTileZ < 0)
	{
		--Result.ChunkZ;
	}

	Result.Offset_.X = (real32)((AbsTileX - TILES_PER_CHUNK/2) - (Result.ChunkX * TILES_PER_CHUNK)) * World->TileSideInMeters;
	Result.Offset_.Y = (real32)((AbsTileY - TILES_PER_CHUNK/2) - (Result.ChunkY * TILES_PER_CHUNK)) * World->TileSideInMeters;

	return (Result);

}

inline world_difference
Subtract(world* World, world_position* A, world_position* B) {

	world_difference Result;

	v2 dTileXY = { (real32)A->ChunkX - (real32)B->ChunkX,
				 (real32)A->ChunkY - (real32)B->ChunkY };

	real32 dTileZ = (real32)A->ChunkZ - (real32)B->ChunkZ;

	Result.dXY = World->ChunkSideInMeters*dTileXY + (A->Offset_ - B->Offset_);
	Result.dz = World->ChunkSideInMeters * dTileZ;
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
ChangeEntityLocation(memory_arena *Arena, world* World, uint32 LowEntityIndex,
	world_position* OldP, world_position* NewP)
{
	if (OldP && AreOnSameChunk(World, OldP, NewP))
	{

	}
	else
	{
		if (OldP)
		{
			world_chunk* Chunk = GetWorldChunk(World, NewP->ChunkX, NewP->ChunkY, NewP->ChunkZ, Arena);
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




#if 0

inline world_position
GetChunkPositionFor(world* World, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
	world_position Result;

	Result.ChunkX = AbsTileX >> World->ChunkShift;
	Result.ChunkY = AbsTileY >> World->ChunkShift;
	Result.ChunkZ = AbsTileZ;
	Result.RelTileX = AbsTileX & World->ChunkMask;
	Result.RelTileY = AbsTileY & World->ChunkMask;

	return (Result);
}



inline bool32
IsTileMapPointEmpty(world* World, world_position Pos)
{

	uint32 TileChunkValue = GetTileValue(World, Pos);
	bool32 Empty = IsTileValueEmpty(TileChunkValue);
	return (Empty);
}




/*
	This function checks on the basis of X and Y if we are presnt inside the tile map or not
	so as to find the boundaries of that tile map

*/
inline bool32
IsTileChunkTileEmpty(world* World, world_chunk* WorldChunk, uint32 TestX, uint32 TestY)
{
	bool32 Empty = false;
	if (WorldChunk)
	{
		uint32 TileChunkValue = GetTileValueUnchecked(World, WorldChunk, TestX, TestY);
		Empty = (TileChunkValue == 0);
	}
	return (Empty);
}


inline uint32
GetTileValueUnchecked(world* World, world_chunk* WorldChunk,
	int32 TileX, int32 TileY)
{

	Assert(WorldChunk);
	Assert((TileX < World->ChunkDim));
	Assert(TileY < World->ChunkDim);
	uint32 TileChunkValue = WorldChunk->Tiles[TileY * World->ChunkDim + TileX];
	return (TileChunkValue);
}


inline void
SetTileValueUnchecked(world* World, world_chunk* WorldChunk,
	int32 TileX, int32 TileY, int32 TileValue)
{

	Assert(WorldChunk);
	Assert((TileX < World->ChunkDim));
	Assert(TileY < World->ChunkDim);
	WorldChunk->Tiles[TileY * World->ChunkDim + TileX] = TileValue;
}


inline uint32
GetTileValue(world* World, world_chunk* WorldChunk,
	uint32 TestTileX, uint32 TestTileY)
{
	uint32 TileChunkValue = 0;

	if (WorldChunk && WorldChunk->Tiles)
	{
		TileChunkValue = GetTileValueUnchecked(World, WorldChunk, TestTileX, TestTileY);
	}
	return(TileChunkValue);
}


internal uint32
GetTileValue(world* World, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ) {

	tile_chunk_position ChunkPos = GetChunkPositionFor(World, AbsTileX, AbsTileY, AbsTileZ);
	world_chunk* WorldChunk = GetTileChunk(World, ChunkPos.ChunkX, ChunkPos.ChunkY, ChunkPos.ChunkZ);
	uint32 TileChunkValue = GetTileValue(World, WorldChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);

	return(TileChunkValue);
}

internal uint32
GetTileValue(world* World, world_position Pos) {

	uint32 TileChunkValue = GetTileValue(World, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);

	return(TileChunkValue);
}

inline void
SetTileValue(world* World, world_chunk* WorldChunk,
	uint32 TestTileX, uint32 TestTileY,
	uint32 TileValue) {

	if (WorldChunk && WorldChunk->Tiles) {
		SetTileValueUnchecked(World, WorldChunk, TestTileX, TestTileY, TileValue);
	}
}


internal void
SetTileValue(memory_arena* Arena, world* World,
	uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ,
	uint32 TileValue)
{

	tile_chunk_position ChunkPos = GetChunkPositionFor(World, AbsTileX, AbsTileY, AbsTileZ);
	world_chunk* WorldChunk = GetTileChunk(World, ChunkPos.ChunkX, ChunkPos.ChunkY, ChunkPos.ChunkZ, Arena);
	SetTileValue(World, WorldChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}
#endif

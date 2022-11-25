/*
Date : 20th Nov 2021
Author : Sharun S
Place : Chennai , India
*/

#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define TILE_CHUNK_UNINTITALIZED INT32_MAX


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
			uint32 TileCount = World->ChunkDim * World->ChunkDim;

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
#endif
internal void
InitializeTileMap(world* World, real32 TileSideInMeters)
{
	World->ChunkShift = 4;
	World->ChunkMask = (1 << World->ChunkShift) - 1;
	World->ChunkDim = (1 << World->ChunkShift);
	World->TileSideInMeters = TileSideInMeters;

	for (uint32 WorldChunkIndex = 0;
		WorldChunkIndex < ArrayCount(World->ChunkHash);
		++WorldChunkIndex)
	{
		World->ChunkHash[WorldChunkIndex].ChunkX = TILE_CHUNK_UNINTITALIZED;
	}
}


inline void
ReCannonicalizeCoord(world* World, int32* Tile, real32* TileRel) {

	int32 Offset = RoundReal32ToInt32(*TileRel / World->TileSideInMeters);
	*Tile += Offset;
	*TileRel -= Offset * World->TileSideInMeters;

	//Need to check this Assert hit case
	Assert(*TileRel > -0.5001f * World->TileSideInMeters);
	Assert(*TileRel < 0.5001f * World->TileSideInMeters);

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
MapIntoTileSpace(world* World, world_position BasePos, V2 Offset) {

	world_position Result = BasePos;

	Result.Offset_ += Offset;
	ReCannonicalizeCoord(World, &Result.AbsTileX, &Result.Offset_.X);
	ReCannonicalizeCoord(World, &Result.AbsTileY, &Result.Offset_.Y);

	return (Result);
}

inline bool32
AreOnSameTile(world_position* A, world_position* B) {

	bool32 Result = ((A->AbsTileX == B->AbsTileX) &&
		(A->AbsTileY == B->AbsTileY) &&
		(A->AbsTileZ == B->AbsTileZ));
	return (Result);

}

inline world_difference
Subtract(world* World, world_position* A, world_position* B) {

	world_difference Result;

	V2 dTile = { (real32)A->AbsTileX - (real32)B->AbsTileX,
				(real32)A->AbsTileY - (real32)B->AbsTileY };

	real32 dTileZ = (real32)A->AbsTileZ - (real32)B->AbsTileZ;

	Result.dXY = World->TileSideInMeters * dTile + (A->Offset_ - B->Offset_);
	Result.dz = World->TileSideInMeters * dTileZ;
	return (Result);

}


inline world_position
CenteredTilePoint(uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ) {

	world_position Result = {};

	Result.AbsTileX = AbsTileX;
	Result.AbsTileY = AbsTileY;
	Result.AbsTileZ = AbsTileZ;

	return (Result);
}


#if 0

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

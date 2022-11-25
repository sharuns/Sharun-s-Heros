#pragma once
/*
Date : 25th Nov 2022
Author : Sharun S
Place : Chennai , India
*/

struct world_difference {

	V2 dXY;
	real32 dz;

};

struct world_position {

	// NOTE : sharun : These are fixed point tile location.
	// High bits are the tile chunk index, Low bits
	// are tile index in the chunk
	int32 AbsTileX;
	int32 AbsTileY;
	int32 AbsTileZ;

	V2 Offset_;
};

struct world_entity_block
{
	uint32 EntityCount;
	uint32 LowEntityIndex[16];
	world_entity_block* Next;
};

struct world_chunk {

	int32 ChunkX, ChunkY , ChunkZ;
	world_entity_block FirstBlock;
	world_chunk* NextInHash;
};


struct world {

	real32 TileSideInMeters;
	
	int32 ChunkShift;
	int32 ChunkMask;
	int32 ChunkDim;
	world_chunk ChunkHash[4096];

};
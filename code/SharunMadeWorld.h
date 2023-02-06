/*
Date : 25th Nov 2022
Author : Sharun S
Place : Chennai , India
*/
#pragma once

struct world_position {

	// NOTE : sharun : These are fixed point tile location.
	// High bits are the tile chunk index, Low bits
	// are tile index in the chunk
	int32 ChunkX;
	int32 ChunkY;
	int32 ChunkZ;

	v3 Offset_;
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
	real32 TileDepthInMeters;
	v3 ChunkDimInMeters;
	world_chunk ChunkHash[4096];

	world_entity_block* FirstFree;

};
#pragma once
/*
Date : 20th Nov 2021
Author : Sharun S
Place : Chennai , India 
*/

struct tile_map_difference{

	V2 dXY;
	real32 dz;

};

struct tile_map_position{

	// NOTE : sharun : These are fixed point tile location.
	// High bits are the tile chunk index, Low bits
	// are tile index in the chunk
	uint32 AbsTileX;
	uint32 AbsTileY;
	uint32 AbsTileZ;

	V2 Offset_;
};


struct tile_chunk_position{

	uint32 TileChunkX;
	uint32 TileChunkY;
	uint32 TileChunkZ ;

	uint32 RelTileX;
	uint32 RelTileY;

};

struct tile_chunk{
	uint32 * Tiles;
};


struct tile_map{

	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDim;

	real32 TileSideInMeters;

	uint32 TileChunkCountX;
	uint32 TileChunkCountY;
	uint32 TileChunkCountZ;


	tile_chunk * TileChunks;

};
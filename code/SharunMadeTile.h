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
	int32 AbsTileX;
	int32 AbsTileY;
	int32 AbsTileZ;

	V2 Offset_;
};


struct tile_chunk_position{

	int32 TileChunkX;
	int32 TileChunkY;
	int32 TileChunkZ ;

	int32 RelTileX;
	int32 RelTileY;

};

struct tile_chunk{

	int32 TileChunkX;
	int32 TileChunkY;
	int32 TileChunkZ;

	int32 * Tiles;

	tile_chunk* NextInHash;
};


struct tile_map{

	int32 ChunkShift;
	int32 ChunkMask;
	int32 ChunkDim;

	real32 TileSideInMeters;
	//Starting hash-table approach
	tile_chunk  TileChunksHash[4096];

};
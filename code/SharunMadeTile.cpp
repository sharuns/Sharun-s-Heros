

inline void
ReCannonicalizeCoord(tile_map * TileMap, uint32 * Tile,real32 * TileRel){

	int32 Offset = RoundReal32ToInt32(*TileRel / TileMap->TileSideInMeters);	

	*Tile += Offset; 
	*TileRel -= Offset*TileMap->TileSideInMeters;


	//Need to check this Assert hit case
	Assert(*TileRel >= -0.5f*TileMap->TileSideInMeters);	
	Assert(*TileRel <= 0.5f*TileMap->TileSideInMeters);
	
}



inline tile_map_position 
ReCannonicalizePosition(tile_map * TileMap,tile_map_position Pos){
	
	tile_map_position Result = Pos;

	ReCannonicalizeCoord(TileMap,&Result.AbsTileX, &Result.TileRelX);
	ReCannonicalizeCoord(TileMap,&Result.AbsTileY, &Result.TileRelY);

	return (Result);
}


/*

	Returns the tile in the tileMap whose corordinates have been passed

*/
//===================================>QC
inline uint32
GetTileValueUnchecked(tile_map * TileMap,tile_chunk * TileChunk, 
	uint32 TileX, uint32 TileY){

	Assert(TileChunk);
	Assert((TileX < TileMap->ChunkDim));
	Assert(TileY < TileMap->ChunkDim);
	uint32 TileChunkValue = TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX];
	return (TileChunkValue);

}

//===================================>QC
inline void
SetTileValueUnchecked(tile_map * TileMap,tile_chunk * TileChunk, 
						uint32 TileX, uint32 TileY, uint32 TileValue){

	Assert(TileChunk);
	Assert((TileX < TileMap->ChunkDim));
	Assert(TileY < TileMap->ChunkDim);
	TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX] = TileValue;

}


//===================================>QC
inline tile_chunk*
GetTileChunk(tile_map * TileMap,uint32 TileChunkX, uint32 TileChunkY){

	tile_chunk *  TileChunk = 0;

	if((TileChunkX >= 0 ) && (TileChunkX < TileMap->TileChunkCountX) && 
	   (TileChunkY >= 0 ) && (TileChunkY < TileMap->TileChunkCountY))
	{
		TileChunk = &TileMap->TileChunks[TileChunkY * TileMap->TileChunkCountX + TileChunkX];
		
	}
	return (TileChunk);
}
//===================================>QC
inline tile_chunk_position
GetChunkPositionFor(tile_map * TileMap,uint32 AbsTileX,uint32 AbsTileY){

	tile_chunk_position Result;

	Result.TileChunkX  = AbsTileX >> TileMap->ChunkShift; 
	Result.TileChunkY =  AbsTileY >> TileMap->ChunkShift;
	Result.RelTileX =  AbsTileX & TileMap->ChunkMask;
	Result.RelTileY =  AbsTileY & TileMap->ChunkMask;

	return (Result);
}
//===================================>QC
inline uint32
GetTileValue(tile_map * TileMap,tile_chunk * TileChunk,uint32 TestTileX, uint32 TestTileY){

	uint32 TileChunkValue = 0 ;

	if(TileChunk){
		TileChunkValue = GetTileValueUnchecked(TileMap,TileChunk,TestTileX,TestTileY);
	}
	return(TileChunkValue);
}
//===================================>QC
inline void
SetTileValue(tile_map * TileMap,tile_chunk * TileChunk,uint32 TestTileX, uint32 TestTileY,
			uint32 TileValue){

	uint32 TileChunkValue = 0 ;

	if(TileChunk){
		SetTileValueUnchecked(TileMap,TileChunk,TestTileX,TestTileY,TileValue);
	}
}


internal uint32
GetTileValue(tile_map * TileMap,uint32 AbsTileX, uint32 AbsTileY){

	bool32 Empty = false;
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap,AbsTileX,AbsTileY);
	tile_chunk * TileChunk = GetTileChunk(TileMap,ChunkPos.TileChunkX,ChunkPos.TileChunkY);
	uint32 TileChunkValue = GetTileValue(TileMap,TileChunk,ChunkPos.RelTileX,ChunkPos.RelTileY);

	return(TileChunkValue);
}






/*
	This function checks on the basis of X and Y if we are presnt inside the tile map or not
	so as to find the boundaries of that tile map

*/
inline bool32 
IsTileChunkTileEmpty(tile_map * TileMap, tile_chunk * TileChunk,uint32 TestX, uint32 TestY){

	bool32 Empty = false;
	if(TileChunk){
	
		uint32 TileChunkValue = GetTileValueUnchecked(TileMap,TileChunk,TestX,TestY);
		Empty = (TileChunkValue == 0);
	}
	return (Empty);
}
//===================================>QC
inline bool32 
IsTileMapPointEmpty(tile_map * TileMap, tile_map_position CanPos){
	
		uint32 TileChunkValue = GetTileValue(TileMap,CanPos.AbsTileX,CanPos.AbsTileY);
		bool32 Empty = (TileChunkValue == 0);

	return (Empty);
}


internal void
SetTileValue(memory_arena * Arena ,tile_map * TileMap, 
			uint32 AbsTileX, uint32 AbsTileY,uint32 TileValue) {

	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap,AbsTileX,AbsTileY);
	tile_chunk * TileChunk = GetTileChunk(TileMap,ChunkPos.TileChunkX,ChunkPos.TileChunkY);
	Assert(TileChunk);

	SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY,TileValue);
}

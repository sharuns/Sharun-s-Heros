#pragma once

/*
Date : 3rd Apr 2021
Author : Sharun S
Place : Chennai , India 
*/

#include "SharunMade_Intrinsics.h"
#include "SharunMade_Math.h"
#include "SharunMadeWorld.h"
#include "SharunMade_sim_region.h"
#include "SharunMade_Entity.h"

struct loaded_bitmap{

	int32 Width;
	int32 Height;
	uint32 * Pixels;
};

struct hero_bitmaps{

	v2 Align;
	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso;
};

/*
Any entity present in the game will use this DS
High enitity is one which is being rendered into 
the activbve screen
*/

struct low_entity {

	sim_entity Sim;
	world_position P;

};


struct entity_visible_piece
{
	loaded_bitmap* Bitmap;
	v2 Offset;
	real32 OffsetZ;
	real32 EntityZC;

	real32 R, G, B, A;
	v2 Dim;
};



struct low_entity_chunk_reference
{
	world_chunk *  WorldChunk;
	uint32 EntityIndexChunk;

};

struct controlled_hero
{
	uint32 EntityIndex;
	v2 ddP;
	v2 dSword;
	real32 dZ;
};

struct pairwise_collision_rule 
{
	bool32 CanCollide;
	uint32 StorageIndexA;
	uint32 StorageIndexB;

	pairwise_collision_rule* NextInHash;
};

struct game_state{

	memory_arena WorldArena;
	world * World;

	uint32 CameraFollowingEntityIndex;
	world_position CameraP;

	controlled_hero ControlledHeros[ArrayCount(((game_input *)0)->Controllers)];
	
	uint32 LowEntityCount;
	low_entity LowEntities[100000];

	loaded_bitmap Backdrop;
	loaded_bitmap Shadow;
	loaded_bitmap Stairwell;
	hero_bitmaps HeroBitmaps[4];

	loaded_bitmap Tree;
	loaded_bitmap Sword;

	real32 MetersToPixels;

	pairwise_collision_rule *CollisionRuleHash[256];
	pairwise_collision_rule* FirstFreeCollisionRule;

	sim_entity_collision_volume_group *NullCollision ;
	sim_entity_collision_volume_group *SwordCollision;
	sim_entity_collision_volume_group *StairwellCollision;
	sim_entity_collision_volume_group *PlayerCollision;
	sim_entity_collision_volume_group *MonsterCollision;
	sim_entity_collision_volume_group *WallCollision;
	sim_entity_collision_volume_group* FamiliarCollision;
	sim_entity_collision_volume_group* StandardRoomCollision;

};

struct entity_visible_piece_group
{
	uint32 PieceCount;
	game_state* GameState;
	entity_visible_piece Pieces[32];
};

inline low_entity*
GetLowEntity(game_state* GameState, uint32 Index)
{
	low_entity* Result = 0;
	if ((Index > 0) && (Index < GameState->LowEntityCount))
	{
		Result = GameState->LowEntities + Index;
	}
	return (Result);
}

internal void
AddCollisionRule(game_state* GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);

internal void
ClearCollisionRulesFor(game_state* GameState, uint32 StorageIndex);

/*
Date : 3rd Jan 2023
Author : Sharun S
Place : Alleppey , India
*/

internal sim_entity_hash*
GetHashFromStorageIndex(sim_region* SimRegion, uint32 StorageIndex)
{
	Assert(StorageIndex);
	sim_entity_hash* Result = 0;
	uint32 HashValue = StorageIndex;
	for (uint32 Offset = 0;
		Offset < ArrayCount(SimRegion->Hash);
		++Offset)
	{
		sim_entity_hash* Entry = SimRegion->Hash + ((HashValue + Offset) & (ArrayCount(SimRegion->Hash) - 1));
		if ((Entry->Index == 0) || (Entry->Index == StorageIndex))
		{
			Result = Entry;
			break;
		}
	}
	return (Result);
}

/*internal void
MapStorageIndexToEntity(sim_region* SimRegion, uint32 StorageIndex, sim_entity* Entity)
{
	sim_entity_hash* Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
	Assert((Entry->Index == 0) || (Entry->Index == StorageIndex))
	Entry->Index = StorageIndex;
	Entry->Ptr = Entity;
}*/

inline sim_entity*
GetEntityByStorageIndex(sim_region* SimRegion, uint32 StorageIndex)
{
	sim_entity_hash* Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
	sim_entity* Result = Entry->Ptr;
	return(Result);
}

inline v3
GetSimSpaceP(sim_region* SimRegion, low_entity* Stored)
{
	v3 Result = InvalidP;
	if (!IsSet(&Stored->Sim, EntityFlag_Nonspatial))
	{
		Result = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
	}
	return (Result);
}

internal sim_entity*
AddEntity(game_state* GameState, sim_region* SimRegion, uint32 StorageIndex, low_entity* Source, v3* SimP);

inline void
LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference* Ref)
{
	if (Ref->Index)
	{
		sim_entity_hash* Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
		if (Entry->Ptr == 0)
		{
			Entry->Index = Ref->Index;
			low_entity* LowEntity = GetLowEntity(GameState, Ref->Index);
			v3 P = GetSimSpaceP(SimRegion, LowEntity);
			Entry->Ptr = AddEntity(GameState, SimRegion, Ref->Index, LowEntity,&P);
		}
		Ref->Ptr = Entry->Ptr;
	}
}

inline void
StoreEntityReference(entity_reference* Ref)
{
	if (Ref->Ptr != 0)
	{
		Ref->Index = Ref->Ptr->StorageIndex;
	}
}

internal sim_entity*
AddEntityRaw(game_state *GameState, sim_region * SimRegion, uint32 StorageIndex, low_entity * Source)
{	
	Assert(StorageIndex);
	sim_entity* Entity = 0;

	sim_entity_hash* Entry = GetHashFromStorageIndex(SimRegion,StorageIndex);
	if (Entry->Ptr == 0)
	{
		if (SimRegion->EntityCount < SimRegion->MaxEntityCount)
		{
			Entity = SimRegion->Entities + SimRegion->EntityCount++;
			Entry->Index = StorageIndex;
			Entry->Ptr = Entity;

			if (Source)
			{
				*Entity = Source->Sim;
				LoadEntityReference(GameState, SimRegion, &Entity->Sword);
			}

			Entity->StorageIndex = StorageIndex;
			Entity->Updatable = false;

		}
		else
		{
			InvalidCodePath;
		}
	}

	return (Entity);
}

internal bool32
EntityOverlapsRectangle(v3 P, sim_entity_collision_volume Volume, rectangle3 Rect)
{
	rectangle3 Grown = AddRadiusTo(Rect, 0.5f *Volume.Dim);
	bool32 Result = IsInRectangle(Grown, P + Volume.OffsetP);
	return (Result);
}

internal sim_entity *
AddEntity(game_state * GameState,sim_region* SimRegion,uint32 StorageIndex, low_entity* Source, v3 * SimP)
{
	sim_entity* Dest = AddEntityRaw(GameState,SimRegion, StorageIndex, Source);
	if (Dest)
	{
		if (SimP)
		{
			Dest->P = *SimP;
			Dest->Updatable =  EntityOverlapsRectangle( Dest->P, Dest->Collision->TotalVolume, SimRegion->UpdatableBounds);
		}
		else
		{
			Dest->P = GetSimSpaceP(SimRegion, Source);
		}
	}
	return (Dest);
}




internal sim_region * 
BeginSim(memory_arena *SimArena,game_state *GameState, world * World ,world_position Origin, rectangle3 Bounds, real32 dt)
{
	sim_region* SimRegion = PushStruct(SimArena, sim_region);
	ZeroStruct(SimRegion->Hash);

	SimRegion->MaxEntityRadius = 5.0f;
	SimRegion->MaxEntityVelocity = 30.0f;
	real32 UpdateSafetyMargin = SimRegion->MaxEntityRadius + SimRegion->MaxEntityVelocity *dt;
	real32 UpdateSafetyMarginZ = 1.0f;

	SimRegion->World = World;
	SimRegion->Origin = Origin;
	SimRegion->UpdatableBounds = AddRadiusTo(Bounds, V3(SimRegion->MaxEntityRadius, SimRegion->MaxEntityRadius, SimRegion->MaxEntityRadius));
	SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatableBounds,
									V3(UpdateSafetyMargin, UpdateSafetyMargin, UpdateSafetyMarginZ));

	SimRegion->MaxEntityCount = 4096;
	SimRegion->EntityCount = 0;
	SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);

	world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
	world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));

	for (int32 ChunkZ = MinChunkP.ChunkZ;
		ChunkZ <= MaxChunkP.ChunkZ;
		++ChunkZ)
	{

		for (int32 ChunkY = MinChunkP.ChunkY;
			ChunkY <= MaxChunkP.ChunkY;
			++ChunkY)
		{
			for (int32 ChunkX = MinChunkP.ChunkX;
				ChunkX <= MaxChunkP.ChunkX;
				++ChunkX)
			{
				world_chunk* Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
				if (Chunk)
				{
					for (world_entity_block* Block = &Chunk->FirstBlock;
						Block;
						Block = Block->Next)
					{
						for (uint32 EntityIndexIndex = 0;
							EntityIndexIndex < Block->EntityCount;
							++EntityIndexIndex)
						{
							uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
							low_entity* Low = GameState->LowEntities + LowEntityIndex;
							if (!IsSet(&Low->Sim, EntityFlag_Nonspatial))
							{
								v3 SimSpaceP = GetSimSpaceP(SimRegion, Low);

								if (EntityOverlapsRectangle(SimSpaceP, Low->Sim.Collision->TotalVolume, SimRegion->Bounds))
								{
									AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
								}
							}
						}
					}
				}
			}
		}
	}
	return (SimRegion);
}

internal void 
EndSim(sim_region* Region, game_state *GameState)
{
	sim_entity *Entity = Region->Entities;
	for (uint32 EntityIndex = 0;
		EntityIndex < Region->EntityCount;
		++EntityIndex, ++Entity)
	{
		if (Entity->Type == EntityType_Hero)
		{
			int r = 10;
		}
		//Store Entity
		low_entity* Stored = GameState->LowEntities + Entity->StorageIndex;
		Stored->Sim = *Entity;

		StoreEntityReference(&Stored->Sim.Sword);
		world_position NewP = IsSet(Entity , EntityFlag_Nonspatial) ?
							  NullPosition() :
							  MapIntoChunkSpace(GameState->World, Region->Origin, Entity->P);
		ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->StorageIndex, Stored, NewP);

		if(Entity->StorageIndex == GameState->CameraFollowingEntityIndex)
		{
			world_position NewCameraP = GameState->CameraP;

			NewCameraP.ChunkZ = Stored->P.ChunkZ;
#if 0
			if (CameraFollowingEntity.High->P.X > (9.0f * World->TileSideInMeters))
			{
				NewCameraP.ChunkX += 17;
			}
			if (CameraFollowingEntity.High->P.X < -(9.0f * World->TileSideInMeters))
			{
				NewCameraP.ChunkX -= 17;
			}
			if (CameraFollowingEntity.High->P.Y > (5.0f * World->TileSideInMeters))
			{
				NewCameraP.ChunkY += 9;
			}
			if (CameraFollowingEntity.High->P.Y < -(5.0f * World->TileSideInMeters))
			{
				NewCameraP.ChunkY -= 9;
			}
#else
			real32 CamZOffset = NewCameraP.Offset_.z;
			NewCameraP = Stored->P;
			NewCameraP.Offset_.z = CamZOffset;
#endif
			GameState->CameraP = NewCameraP;
		}
	}
}

struct test_wall
{
	real32 X;
	real32 RelX;
	real32 RelY;
	real32 DeltaX;
	real32 DeltaY;
	real32 MinY;
	real32 MaxY;
	v3 Normal;
};

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
				*tMin = Maximum(0.0f, tResult - tEpsilon);
				Hit = true;
			}
		}
	}
	return (Hit);
}


internal bool32 
HandleCollision(game_state *GameState, sim_entity* A, sim_entity* B)
{
	bool32 StopsOnCollision = false;

	if (A->Type == EntityType_Sword)
	{
		AddCollisionRule(GameState, A->StorageIndex, B->StorageIndex, false);
		StopsOnCollision = false;
	}
	else
	{
		StopsOnCollision = true;
	}


	if (A->Type > B->Type)
	{
		sim_entity* Temp = A;
		A = B;
		B = Temp;
	}
	if ((A->Type == EntityType_Monster) &&
		(B->Type == EntityType_Sword))
	{
		if (A->HitPointMax > 0)
		{
			--A->HitPointMax;
		}
	}

	return (StopsOnCollision);
}


internal bool32
CanCollide(game_state *GameState, sim_entity* A, sim_entity* B)
{
	bool32 Result = false;

	if (A!= B)
	{
		if (A->StorageIndex > B->StorageIndex)
		{
			sim_entity* Temp = A;
			A = B;
			B = Temp;
		}

		if (IsSet(A, EntityFlag_Collides) && IsSet(B, EntityFlag_Collides))
		{
			if (!IsSet(A, EntityFlag_Nonspatial) &&
				!IsSet(B, EntityFlag_Nonspatial))
			{
				Result = true;
			}

			uint32 HashBucket = A->StorageIndex & (ArrayCount(GameState->CollisionRuleHash) - 1);
			for (pairwise_collision_rule* Rule = GameState->CollisionRuleHash[HashBucket];
				Rule;
				Rule = Rule->NextInHash)
			{
				if ((Rule->StorageIndexA == A->StorageIndex) &&
					(Rule->StorageIndexB == B->StorageIndex))
				{
					Result = Rule->CanCollide;
					break;
				}
			}
		}
	}
	return (Result);
}


internal bool32 
CanOverlap(game_state* GameState, sim_entity* Mover, sim_entity* Region)
{
	bool32 Result = false;

	if(Mover != Region)
	{ 
		if (Region->Type == EntityType_Stairwell)
		{
			Result = true;
		}
	}

	return(Result);

}

internal void
HandleOverlap(game_state* GameState, sim_entity* Mover, sim_entity* Region, real32 dt,
			  real32 *Ground)
{
	if (Region->Type == EntityType_Stairwell)
	{
		*Ground = GetStairGround(Region, GetEntityGroundPoint(Mover));
	}
}

internal bool32
SpeculativeCollide(sim_entity* Mover, sim_entity* Region, v3 TestP)
{
	bool32 Result = true;

	if (Region->Type == EntityType_Stairwell)
	{
		real32 StepHeight = 0.1f;
#if 0
		Result = ((AbsoluteValue(GetEntityGroundPoint(Mover).Z - Ground) > StepHeight) ||
			((Bary.Y > 0.1f) && (Bary.Y < 0.9f)));
#endif
		v3 MoverGroundPoint = GetEntityGroundPoint(Mover, TestP);
		real32 Ground = GetStairGround(Region, MoverGroundPoint);
		Result = (AbsoluteValue(MoverGroundPoint.z - Ground) > StepHeight);

	}
	return (Result);
}

internal bool32 
EntitiesOverlap(sim_entity * Entity, sim_entity * TestEntity, v3 Epsilon = V3(0,0,0))
{
	bool32 Result = false;

	for (uint32 VolumeIndex = 0;
		!Result && (VolumeIndex < Entity->Collision->VolumeCount);
		VolumeIndex++)
	{
		sim_entity_collision_volume* Volume =
			Entity->Collision->Volumes + VolumeIndex;
		for (uint32 TestVolumeIndex = 0;
			!Result && (TestVolumeIndex < TestEntity->Collision->VolumeCount);
			++TestVolumeIndex)
		{
			sim_entity_collision_volume* TestVolume =
				TestEntity->Collision->Volumes + TestVolumeIndex;

			rectangle3 EntityRect = RectCenterDim(Entity->P + Volume->OffsetP, Volume->Dim + Epsilon);

			rectangle3 TestEntityRect = RectCenterDim(TestEntity->P + TestVolume->OffsetP, TestVolume->Dim);
			Result = RectangleIntersect(EntityRect, TestEntityRect);
		}
	}

	return (Result);
}



internal void
MoveEntity(game_state *GameState, sim_region *SimRegion, sim_entity *Entity, real32 dt , move_spec* MoveSpec, v3 ddP )
{
	world* World = SimRegion->World;

	if (MoveSpec->UnitMaxAccelVector)
	{
		real32 ddPLength = LengthSq(ddP);
		if (ddPLength > 1.0f) {

			ddP *= (1.0f / SquareRoot(ddPLength));
		}
	}

	//This is default acceleration value of the player

	ddP *= MoveSpec->Speed;

	v3 Drag = -MoveSpec->Drag * Entity->dP;
	Drag.z = 0.0f;
	ddP += Drag;
	if(!IsSet(Entity,EntityFlag_ZSupported))
	{
		ddP += V3(0.0f, 0.0f, -9.8f);
	}
	

	v3 OldPlayerP = Entity->P;
	v3 PlayerDelta = (0.5f * ddP * Square(dt) + Entity->dP * dt);
	// velocity equation v' = (at + v)
	Entity->dP = ddP * dt + Entity->dP;
	Assert(LengthSq(Entity->dP) <= Square(SimRegion->MaxEntityVelocity));
	v3 NewPlayerP = OldPlayerP + PlayerDelta;

	real32 DistanceRemaining = Entity->DistanceLimit;
	if (DistanceRemaining == 0.0f)
	{
		DistanceRemaining  = 10000.0f;
	}

	uint32 OverlappingCount = 0;
	//sim_entity* OverLappingEntities[16];

	//position equation p' = (1/2 * at^2 + Vt + p)
	/*collision detection : based on the Basic Minkowski collision detection*/
	for (uint32 Iterations = 0;
		Iterations < 4;
		++Iterations)
	{
		real32 tMin = 1.0f;
		//real32 tMax = 0.0f;
		real32 tMax = 1.0f;

		real32 PlayerDeltaLength = Length(PlayerDelta);
		if (PlayerDeltaLength > 0.0f)
		{
			if (PlayerDeltaLength > DistanceRemaining)
			{
				tMin = DistanceRemaining / PlayerDeltaLength;
			}

			v3 WallNormalMin = {};
			v3 WallNormalMax = {};
			sim_entity* HitEntityMin = 0;
			sim_entity* HitEntityMax = 0;

			v3 DesiredPosition = Entity->P + PlayerDelta;

			if (!IsSet(Entity, EntityFlag_Nonspatial))
			{
				for (uint32 TestHighEntityIndex = 0;
					TestHighEntityIndex < SimRegion->EntityCount;
					++TestHighEntityIndex)
				{
					sim_entity* TestEntity = SimRegion->Entities + TestHighEntityIndex;
					real32 OverlapEpsilon = 0.001f;
					if(IsSet(TestEntity, EntityFlag_Traversable) && 
					EntitiesOverlap(Entity, TestEntity, V3(OverlapEpsilon, OverlapEpsilon, OverlapEpsilon)) ||
					(CanCollide(GameState, Entity, TestEntity)))
					{
						for (uint32 VolumeIndex = 0 ; 
							VolumeIndex < Entity->Collision->VolumeCount; 
							VolumeIndex++)
						{
							sim_entity_collision_volume* Volume = 
								Entity->Collision->Volumes + VolumeIndex;
							for(uint32 TestVolumeIndex = 0 ;
								TestVolumeIndex < TestEntity->Collision->VolumeCount;
								++TestVolumeIndex)
							{ 
								sim_entity_collision_volume* TestVolume =
									TestEntity->Collision->Volumes + TestVolumeIndex;
								v3 MinkowskiDiameter = { TestVolume->Dim.x + Volume->Dim.x,
														 TestVolume->Dim.y + Volume->Dim.y,
														 TestVolume->Dim.z + Volume->Dim.z };

								v3 MinCorner = -0.5f * MinkowskiDiameter;
								v3 MaxCorner = 0.5f * MinkowskiDiameter;

								v3 Rel = ((Entity->P + Volume->OffsetP) - (TestEntity->P + TestVolume->OffsetP));

								if ((Rel.z >= MinCorner.z) && (Rel.z < MaxCorner.z))
								{
#if 0
									
									if (TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
										&tMinTest, MinCorner.Y, MaxCorner.Y))
									{

										TestWallNormal = v3{ -1,0,0 };
										HitThis = true;
									}

									if (TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y,
										&tMinTest, MinCorner.Y, MaxCorner.Y))
									{
										TestWallNormal = v3{ 1,0,0 };
										HitThis = true;
									}

									if (TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
										&tMinTest, MinCorner.X, MaxCorner.X))
									{
										TestWallNormal = v3{ 0,-1,0 };
										HitThis = true;
									}

									if (TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X,
										&tMinTest, MinCorner.X, MaxCorner.X))
									{
										TestWallNormal = v3{ 0,1,0 };
										HitThis = true;
									}

#endif
									test_wall Walls[] = {

										{MinCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y, MaxCorner.y, V3(-1,0,0)},
										{MaxCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y, MaxCorner.y, V3(1,0,0)},
										{MinCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x, MaxCorner.x, V3(0,-1,0)},
										{MaxCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x, MaxCorner.x, V3(0,1,0)},
									};

									if (IsSet(TestEntity, EntityFlag_Traversable))
									{
										

											//real32 tMinTest = tMin;
											real32 tMaxTest = tMax;
											bool32 HitThis = false;

											v3 TestWallNormal = {};
											for (uint32 WallIndex = 0;
												WallIndex < ArrayCount(Walls);
												++WallIndex)
											{
												test_wall* Wall = Walls + WallIndex;

												real32 tEpsilon = 0.001f;
												if (Wall->DeltaX != 0.0f)
												{
													real32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
													real32 Y = Wall->RelY + tResult * Wall->DeltaY;
													if ((tResult <= 0.0f) && (tMaxTest < tResult))
													{
														if ((Y >= Wall->MinY) && (Y <= Wall->MaxY))
														{
															tMaxTest = Maximum(0.0f, tResult - tEpsilon);
															TestWallNormal = Wall->Normal;
															HitThis = true;
														}
													}
												}

											}
											if (HitThis)
											{

												tMax = tMaxTest;
												WallNormalMax = TestWallNormal;
												HitEntityMax = TestEntity;


											}
										
									}
									else

									{
										real32 tMinTest = tMin;
										bool32 HitThis = false;

										v3 TestWallNormal = {};
										for (uint32 WallIndex = 0;
											WallIndex < ArrayCount(Walls);
											++WallIndex)
										{
											test_wall* Wall = Walls + WallIndex;

											real32 tEpsilon = 0.001f;
											if (Wall->DeltaX != 0.0f)
											{
												real32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
												real32 Y = Wall->RelY + tResult * Wall->DeltaY;
												if ((tResult >= 0.0f) && (tMinTest > tResult))
												{
													if ((Y >= Wall->MinY) && (Y <= Wall->MaxY))
													{
														tMinTest = Maximum(0.0f, tResult - tEpsilon);
														TestWallNormal = Wall->Normal;
														HitThis = true;
													}
												}
											}

										}
										if (HitThis)
										{
											v3 TestP = Entity->P + tMinTest * PlayerDelta;
											if (SpeculativeCollide(Entity, TestEntity, TestP))
											{
												tMin = tMinTest;
												WallNormalMin = TestWallNormal;
												HitEntityMin = TestEntity;
											}

										}
									}
									
								}

							}
						}
					}
				}
			}
			//end of collision dectection
			v3 WallNormal = {};
			sim_entity* HitEntity = 0;
			real32 tStop;
			if (tMin < tMax)
			{
				tStop = tMin;
				HitEntity = HitEntityMin;
				WallNormal = WallNormalMin;
			}
			else
			{
				tStop = tMax;
				HitEntity = HitEntityMax;
				WallNormal = WallNormalMax;
			}
			Entity->P += tStop * PlayerDelta;
			DistanceRemaining -= tStop*PlayerDeltaLength;

			if (HitEntity)
			{
				PlayerDelta = DesiredPosition - Entity->P;

				//bool32 WasOverlapping = (OverlapIndex != OverlappingCount);
				bool32 StopsOnCollision = HandleCollision(GameState, Entity, HitEntity);

				if (StopsOnCollision)
				{
					PlayerDelta = PlayerDelta - 1*Inner(PlayerDelta, WallNormal) * WallNormal;
					Entity->dP = Entity->dP - 1*Inner(Entity->dP, WallNormal) * WallNormal;
				}
				//Entity->dAbsTileZ += HitEntity->dAbsTileZ;
			}
			else
			{
				break;
			}

		}
		else
		{
			break;
		}
	}

	real32 Ground = 0.0f;	// SimRegion->GroundZBase;

	{
	
		for (uint32 TestHighEntityIndex = 0;
			TestHighEntityIndex < SimRegion->EntityCount;
			++TestHighEntityIndex)
		{
			sim_entity* TestEntity = SimRegion->Entities + TestHighEntityIndex;
			if (CanOverlap(GameState, Entity, TestEntity) && (EntitiesOverlap(Entity, TestEntity)))
			{
					HandleOverlap(GameState, Entity, TestEntity, dt, &Ground);
			}
		}
	}

	Ground += Entity->P.z - GetEntityGroundPoint(Entity).z;
	if ((Entity->P.z <= Ground) ||
		(IsSet(Entity, EntityFlag_ZSupported)) && 
		(Entity->dP.z == 0.0f))
	{
		Entity->P.z = Ground;
		Entity->dP.z = 0;
		AddFlags(Entity, EntityFlag_ZSupported);
	}
	else
	{
		ClearFlags(Entity, EntityFlag_ZSupported);
	}
	//position equation p' = (1/2 * at^2 + Vt + p)
	if (Entity->DistanceLimit != 0.0f)
	{
		Entity->DistanceLimit = DistanceRemaining;
	}

	if (Entity->dP.x == 0.0f && (Entity->dP.y == 0.0f)) {

	}
	else if (AbsoluteValue(Entity->dP.x) > AbsoluteValue(Entity->dP.y))
	{

		if (Entity->dP.x > 0)
		{
			Entity->FacingDirection = 0; //[ssk] : Right facing
		}
		else
		{
			Entity->FacingDirection = 2; //[ssk] : Left facing
		}

	}
	else
	{

		if (Entity->dP.y > 0)
		{
			Entity->FacingDirection = 1; //[ssk] : Back facing
		}
		else
		{
			Entity->FacingDirection = 3; ////[ssk] : Front facing
		}

	}
}
/*
Date : 4th Jan 2023
Author : Sharun S
Place : Alleppey , India
*/

inline move_spec
DefaultMoveSpec(void)
{
	move_spec Result;

	Result.UnitMaxAccelVector = false;
	Result.Drag = 0.0f;
	Result.Speed = 1.0f;

	return (Result);
}

inline void
UpdateFamiliar(sim_region * SimRegion, sim_entity *Entity, real32 dt)
{
	sim_entity * ClosestHero = 0;
	real32 ClosestHeroDSq = Square(10.0f);

	sim_entity* TestEntity = SimRegion->Entities;
	for (uint32 TestEntityIndex = 0;
		TestEntityIndex < SimRegion->EntityCount;
		++TestEntityIndex)
	{

		if (TestEntity->Type == EntityType_Hero)
		{
			real32 TestDSq = LengthSq(TestEntity->P - Entity->P);
			if (TestEntity->Type == EntityType_Hero)
			{
				TestDSq *= 0.75f;
			}
			if (ClosestHeroDSq > TestDSq)
			{
				ClosestHero = TestEntity;
				ClosestHeroDSq = TestDSq;
			}
		}
	}

	v2 ddP = {};
	if (ClosestHero && (ClosestHeroDSq > Square(2.0f)))
	{
		real32 Acceleration = 0.5f;
		real32 OneOverLength = (Acceleration / SquareRoot(ClosestHeroDSq));
		ddP = OneOverLength * (ClosestHero->P - Entity->P);
	}
	move_spec MoveSpec = DefaultMoveSpec();
	MoveSpec.UnitMaxAccelVector = true;
	MoveSpec.Drag = 8.0f;
	MoveSpec.Speed = 50.0f;
	MoveEntity(SimRegion, Entity, dt, &MoveSpec, ddP);
}

inline void
UpdateMonster(sim_region* SimRegion, sim_entity Entity, real32 dt)
{
}

inline void
UpdateSword(sim_region* SimRegion, sim_entity * Entity, real32 dt)
{
	if (IsSet(Entity, EntityFlag_Nonspatial))
	{
	}
	else
	{
		move_spec MoveSpec = DefaultMoveSpec();
		MoveSpec.UnitMaxAccelVector = false;
		MoveSpec.Drag = 0.0f;
		MoveSpec.Speed = 0.0f;

		v2 OldP = Entity->P;
		MoveEntity(SimRegion, Entity, dt, &MoveSpec, V2(0, 0));
		real32 DistanceTraveled = Length(Entity->P - OldP);

		Entity->DistanceRemaining -= DistanceTraveled;
		if (Entity->DistanceRemaining < 0)
		{
			MakeEntityNonSpatial(Entity);
		}
	}
}
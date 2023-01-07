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
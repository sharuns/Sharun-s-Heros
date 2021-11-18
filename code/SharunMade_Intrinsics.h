#pragma once

#include"math.h"
#define RoundingFraction 0.5f

//===============================================================
//!
//! note : Method to Round a floating point number to integer
//! 
//! args : float value
//!
internal int32
RoundReal32ToInt32(real32 Real32) {

	/*
		0.75f + 0.5f = 1.25f => on truncating becomes 1
		0.25f + 0.5f = 0.75f => on truncating becomes 0
		This operation mimics Rounding using tuncation
	*/
	int32 Result = (int32)(Real32 + RoundingFraction);
	return (Result);

}

//===============================================================
//!
//! note : Method to Round a floating point number to unsigned integer 
//! 
//! args : float value
//!
internal uint32
RoundReal32ToUint32(real32 Real32) {

	/*
		0.75f + 0.5f = 1.25f => on truncating becomes 1
		0.25f + 0.5f = 0.75f => on truncating becomes 0
		This operation mimics Rounding using tuncation
	*/
	uint32 Result = (uint32)(Real32 + RoundingFraction);
	return (Result);

}


internal int32
FloorReal32ToInt32(real32 Real32) {
	int32 Result = (int32)floorf(Real32);
	return (Result);
}

internal int32
TruncateReal32ToInt32(real32 Real32) {

	int32 Result = (int32)Real32;
	return (Result);
}

inline real32
Sin(real32 Angle) {

	real32 Result = sinf(Angle);
	return Result;
}

inline real32
Cos(real32 Angle) {

	real32 Result = cosf(Angle);
	return Result;

}

inline real32
Atan2(real32 Y, real32 X) {
	real32 Result = atan2f(Y, X);
		return Result;
}
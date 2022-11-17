#pragma once

union V2{

	struct {
		real32 X,Y;

	};

	real32 E[2];

};


inline V2 v2(real32 X, real32 Y){

	V2 Result;

	Result.X = X;
	Result.Y = Y;

	return Result;
}

inline
V2 operator*(real32 A, V2 B){
	V2 Result;
	Result.X = A*B.X;
	Result.Y = A*B.Y;
	return Result;
}

inline
V2 operator*(V2 B, real32 A ){
	V2 Result = A*B;
	
	return Result;
}

inline 
V2 & operator*=(V2 &B,real32 A){

	B = A * B;
	return (B);
	
}


inline
V2 operator-(V2 A){

	V2 Result ;
	Result.X = -A.X;
	Result.Y = -A.Y;

	return Result;

}

inline
V2 operator+( V2 A, V2 B){

	V2 Result ;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return (Result);

}

inline 
V2& operator+=( V2 &A,V2 B){

	A = A+B;
	return A;

}

inline
V2 operator-(V2 A, V2 B){

	V2 Result ;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return (Result);
}

inline real32
Square(real32 A){

	real32 Result = A*A;
	return Result;

}
inline real32
Inner(V2 A,V2 B){

	real32 Result = A.X*B.X + A.Y*B.Y;
	return Result;
}


inline real32
LengthSq(V2 A){

	real32 Result = Inner(A,A);
	return Result;

}

struct rectangle2
{
	V2 Min;
	V2 Max;
};

inline rectangle2
RectMinDim(V2 Min, V2 Dim) 
{
	rectangle2 Result;

	Result.Min = Min;
	Result.Max = Min + Dim;

	return (Result);

}


inline rectangle2
RectCenterHalfDim(V2 Center, V2 HalfDim) 
{
	rectangle2 Result;

	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return (Result);
}

inline rectangle2
RectCenterDim(V2 Center, V2 Dim) 
{
	rectangle2 Result = RectCenterHalfDim(Center, 0.5 * Dim);
	return (Result);
}

inline bool32
IsInRectangle(rectangle2 Rectangle, V2 Test) 
{
	bool32 Result = ((Test.X >= Rectangle.Min.X) &&
		(Test.Y >= Rectangle.Min.Y) &&
		(Test.X < Rectangle.Max.X) &&
		(Test.Y < Rectangle.Max.Y));
	return (Result);
}




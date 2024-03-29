#pragma once

//2-D Vector 
union v2{
	struct {
		real32 X,Y;
	};
	real32 E[2];
};


//3-D vector
union v3 {
	struct {
		real32 X, Y, Z;
	};
	struct {
		real32 R, G, B;
	};
	struct
	{
		v2 XY;
		real32 _Ignored;
	};
	real32 E[3];
};

//4-D vector
union v4 {
	struct {
		real32 X, Y, Z, W;
	};
	struct {
		real32 R, G, B, A;
	};
	real32 E[4];
};

struct rectangle2
{
	v2 Min;
	v2 Max;
};

struct rectangle3
{
	v3 Min;
	v3 Max;
};

inline v2
V2i(int32 X, int32 Y)
{
	v2 Result = { (real32)X,(real32)Y };
	return (Result);
}

inline v2
V2u(uint32 X, uint32 Y)
{
	v2 Result = { (real32)X,(real32)Y };
	return (Result);
}

inline v2 V2(real32 X, real32 Y){
	v2 Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

inline v3 V3(real32 X, real32 Y, real32 Z) {
	v3 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

inline v3
V3(v2 XY, real32 Z) 
{
	v3 Result;
	Result.X = XY.X;
	Result.Y = XY.Y;
	Result.Z = Z;

	return Result;
}

inline real32
Clamp(real32 Min, real32 Value, real32 Max)
{
	real32 Result = Value;
	if (Result < Min)
	{
		Result = Min;
	}
	else if (Result > Max)
	{
		Result = Max;
	}
	return (Result);
}

inline real32
Clamp01(real32 Value)
{
	real32  Result = Clamp(0.0f, Value, 1.0f);
	return Result;

}

inline v3
Clamp01(v3 Value)
{
	v3  Result;
	Result.X = Clamp01(Value.X);
	Result.Y = Clamp01(Value.Y);
	Result.Z = Clamp01(Value.Z);
	return Result;

}

inline v2
Clamp01(v2 Value)
{
	v2  Result;
	Result.X = Clamp01(Value.X);
	Result.Y = Clamp01(Value.Y);
	return Result;
}


inline v4 V4(real32 X, real32 Y, real32 Z, real32 W) {
	v4 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;
	return Result;
}

//Scalar operators 
inline real32
Square(real32 A) 
{
	real32 Result = A * A;
	return Result;
}

inline real32 
Lerp(real32 A, real32 t, real32 B)
{
	real32 Result = (1.0f - t) * A + t * B;
	return Result;
}





// V2 operations

inline
v2 operator*(real32 A, v2 B){
	v2 Result;
	Result.X = A*B.X;
	Result.Y = A*B.Y;
	return Result;
}

inline
v2 operator*(v2 B, real32 A ){
	v2 Result = A*B;
	
	return Result;
}

inline 
v2 & operator*=(v2 &B,real32 A){

	B = A * B;
	return (B);
	
}


inline
v2 operator-(v2 A){

	v2 Result ;
	Result.X = -A.X;
	Result.Y = -A.Y;

	return Result;

}

inline
v2 operator+( v2 A, v2 B){

	v2 Result ;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return (Result);

}

inline 
v2& operator+=( v2 &A,v2 B){

	A = A+B;
	return A;

}

inline
v2 operator-(v2 A, v2 B){

	v2 Result ;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return (Result);
}


//This is the Dot Product 
inline real32
Inner(v2 A,v2 B)
{
	real32 Result = A.X*B.X + A.Y*B.Y;
	return Result;
}

//This is the Hadamard Product 
inline v2
Hadamard(v2 A, v2 B)
{
	v2 Result = { A.X * B.X , A.Y * B.Y };
	return Result;
}

inline real32
LengthSq(v2 A){

	real32 Result = Inner(A,A);
	return Result;

}


inline real32
Length(v2 A) {

	real32 Result = SquareRoot(LengthSq(A));
	return Result;

}
// V3 operations 
inline v3
operator*(real32 A, v3 B)
{
	v3 Result = {};

	Result.X = A * B.X;
	Result.Y = A * B.Y;
	Result.Z = A * B.Z;
	return Result;
}

inline v3 
operator*(v3 B, real32 A) 
{
	v3 Result = A * B;
	return Result;
}

inline v3 & 
operator*=(v3& B, real32 A)
{
	B = A * B;
	return (B);
}

inline v3
operator-(v3 A)
{
	v3 Result;
	Result.X = -A.X;
	Result.Y = -A.Y;
	Result.Z = -A.Z;

	return Result;
}

inline v3 
operator+(v3 A, v3 B) 
{
	v3 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	Result.Z = A.Z + B.Z;

	return (Result);
}

inline v3 & 
operator+=(v3 &A, v3 B)
{
	A = A + B;
	return A;
}

inline v3 
operator-(v3 A, v3 B)
{
	v3 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	Result.Z = A.Z - B.Z;

	return (Result);
}


//This is the Dot Product 
inline real32
Inner(v3 A, v3 B)
{
	real32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
	return Result;
}

//This is the Hadamard Product 
inline v3
Hadamard(v3 A, v3 B)
{
	v3 Result = { A.X * B.X , A.Y * B.Y, A.Z * B.Z };
	return Result;
}

inline real32
LengthSq(v3 A)
{
	real32 Result = Inner(A, A);
	return Result;
}


inline real32
Length(v3 A) 
{
	real32 Result = SquareRoot(LengthSq(A));
	return Result;
}
//


// Rectangle 2 operations 
inline v2
GetMinCorner(rectangle2 Rect)
{
	v2 Result = Rect.Min;
	return (Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
	v2 Result = Rect.Max;
	return (Result);

}

inline v2 GetCenter(rectangle2 Rect)
{
	v2 Result = 0.5f * (Rect.Min + Rect.Max);
	return Result;
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim) 
{
	rectangle2 Result;

	Result.Min = Min;
	Result.Max = Min + Dim;

	return (Result);

}


inline real32
SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
	real32 Result = N;
	if (Divisor != 0.0f)
	{
		Result = Numerator / Divisor;
	}

	return  (Result);
}

inline real32
safeRatio0(real32 Numerator, real32 Divisor)
{
	real32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
	return  (Result);
}

inline real32
safeRatio1(real32 Numerator, real32 Divisor)
{
	real32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
	return  (Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim) 
{
	rectangle2 Result;

	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return (Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim) 
{
	rectangle2 Result = RectCenterHalfDim(Center, 0.5 * Dim);
	return (Result);
}

inline bool32
IsInRectangle(rectangle2 Rectangle, v2 Test) 
{
	bool32 Result = ((Test.X >= Rectangle.Min.X) &&
		(Test.Y >= Rectangle.Min.Y) &&
		(Test.X < Rectangle.Max.X) &&
		(Test.Y < Rectangle.Max.Y));
	return (Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
	rectangle2 Result;

	Result.Min = A.Min - Radius;
	Result.Max = A.Max + Radius;

	return (Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
	v2 Result;

	Result.X = safeRatio0(P.X - A.Min.X, A.Max.X - A.Min.X);
	Result.Y = safeRatio0(P.Y - A.Min.Y, A.Max.Y - A.Min.Y);

	return (Result);
}



// Rectangle 3 operations 
inline v3
GetMinCorner(rectangle3 Rect)
{
	v3 Result = Rect.Min;
	return (Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
	v3 Result = Rect.Max;
	return (Result);

}

inline v3 GetCenter(rectangle3 Rect)
{
	v3 Result = 0.5f * (Rect.Min + Rect.Max);
	return Result;
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
	rectangle3 Result;

	Result.Min = Min;
	Result.Max = Min + Dim;

	return (Result);

}


inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
	rectangle3 Result;

	Result.Min = Center - HalfDim;
	Result.Max = Center + HalfDim;

	return (Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
	rectangle3 Result = RectCenterHalfDim(Center, 0.5 * Dim);
	return (Result);
}

inline bool32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
	bool32 Result = ((Test.X >= Rectangle.Min.X) &&
					(Test.Y >= Rectangle.Min.Y) &&
					(Test.Z >= Rectangle.Min.Z) &&
					(Test.X < Rectangle.Max.X) &&
					(Test.Y < Rectangle.Max.Y) &&
					(Test.Z < Rectangle.Max.Z));
	return (Result);
}

inline bool32
RectangleIntersect(rectangle3 A, rectangle3 B)
{
	bool32 Result = !((B.Max.X < A.Min.X) ||
					  (B.Min.X > A.Max.X) ||
					  (B.Max.Y < A.Min.Y) ||
					  (B.Min.Y > A.Max.Y) ||
					  (B.Max.Z < A.Min.Z) ||
					  (B.Min.Z > A.Max.Z));
								   
	return (Result);
}


inline v3
GetBarycentric(rectangle3 A, v3 P)
{
	v3 Result;

	Result.X = safeRatio0(P.X - A.Min.X , A.Max.X - A.Min.X);
	Result.Z = safeRatio0(P.Z - A.Min.Z , A.Max.Z - A.Min.Z);
	Result.Y = safeRatio0(P.Y - A.Min.Y , A.Max.Y - A.Min.Y);

	return (Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
	rectangle3 Result;

	Result.Min = A.Min - Radius;
	Result.Max = A.Max + Radius;

	return (Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
	rectangle2 Result;

	Result.Min = A.Min.XY;
	Result.Max = A.Max.XY;

	return(Result);
}



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
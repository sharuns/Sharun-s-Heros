#pragma once

struct V2{

	
	union{

		struct {
			real32 X,Y;

		};
		real32 E[2];
	};

	inline V2& operator*=(real32 A);
	inline V2& operator+=(V2 A);
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
V2 operator+( V2 A, V2 B){

	V2 Result ;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return (Result);

}

inline 
V2 &V2::operator+=( V2 A){

	*this =  *this + A;
	return (*this);

}


inline 
V2 &V2::operator*=(real32 A){

	*this = A *(*this);
	return (*this);
	
}


inline
V2 operator-(V2 A){

	V2 Result ;
	Result.X = -A.X;
	Result.Y = -A.Y;

	return Result;

}









inline
V2 operator-(V2 A, V2 B){

	V2 Result ;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return (Result);
}
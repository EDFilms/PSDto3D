//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file math_2D.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef MATH_2D_H
#define MATH_2D_H

#include <string>

namespace util
{
	//----------------------------------------------------------------------------------------
	struct Vector2F
	{
		float x;
		float y;

		Vector2F();
		Vector2F(float x, float y);
		Vector2F(Vector2F const& v);
		~Vector2F();

		bool operator==(const Vector2F& a) const;
		bool operator< (const Vector2F& rhs) const;
		float operator*(const Vector2F& a) const;
		Vector2F operator+(const Vector2F& a) const;
		Vector2F operator-(const Vector2F& a) const;
		Vector2F operator*(const float& n) const;
		Vector2F operator/(const float& n) const;
		Vector2F& operator+=(const Vector2F& a);
		Vector2F& operator-=(const Vector2F& a);
		Vector2F& operator*=(const float& n);
		Vector2F& operator/=(const float& n);

		float Magnitude() const;
		Vector2F Perpendicular() const;

		std::string ToMString() const;
		static float Magnitude(Vector2F const& v1, Vector2F const& v2);
		static bool AreSimilar(Vector2F const& v1, Vector2F const& v2, float epsilon = 0.001f);
		static Vector2F Abs(Vector2F const& v);
		static Vector2F Mid(Vector2F const & v1, Vector2F const & v2);

		static Vector2F Left;
		static Vector2F Right;
		static Vector2F Up;
		static Vector2F Down;
		static Vector2F Zero;
	};
}

#endif // MATH_2D_H
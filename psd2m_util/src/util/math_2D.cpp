//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file math_2D.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "math_2D.h"

namespace util
{
#pragma region VECTOR2F

	//----------------------------------------------------------------------------------------
	Vector2F::Vector2F()
	{
		this->x = 0.0f;
		this->y = 0.0f;
	}

	//----------------------------------------------------------------------------------------
	Vector2F::Vector2F(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	//----------------------------------------------------------------------------------------
	Vector2F::Vector2F(Vector2F const& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	//----------------------------------------------------------------------------------------
	Vector2F::~Vector2F() = default;

	//----------------------------------------------------------------------------------------
	bool Vector2F::operator==(const Vector2F& a) const
	{
		return (this->x == a.x && this->y == a.y);
	}

	//----------------------------------------------------------------------------------------
	bool Vector2F::operator<(const Vector2F & rhs) const
	{
		return (this->x < rhs.x);
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::operator+(const Vector2F& a) const
	{
		return Vector2F(this->x + a.x, this->y + a.y);
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::operator-(const Vector2F& a) const
	{
		return Vector2F(this->x - a.x, this->y - a.y);
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::operator*(const float& n) const
	{
		return Vector2F(this->x * n, this->y *n);
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::operator/(const float& n) const
	{
		return Vector2F(this->x / n, this->y / n);
	}

	//----------------------------------------------------------------------------------------
	float Vector2F::operator*(const Vector2F& a) const
	{
		return (this->x * a.x) + (this->y * a.y);
	}

	//----------------------------------------------------------------------------------------
	Vector2F& Vector2F::operator+=(const Vector2F& a)
	{
		this->x += a.x;
		this->y += a.y;
		return *this;
	}

	//----------------------------------------------------------------------------------------
	Vector2F& Vector2F::operator-=(const Vector2F& a)
	{
		this->x -= a.x;
		this->y -= a.y;
		return *this;
	}

	//----------------------------------------------------------------------------------------
	Vector2F& Vector2F::operator*=(const float& n)
	{
		this->x *= n;
		this->y *= n;
		return *this;
	}

	//----------------------------------------------------------------------------------------
	Vector2F& Vector2F::operator/=(const float& n)
	{
		this->x /= n;
		this->y /= n;
		return *this;
	}

	float Vector2F::Magnitude() const
	{
		return sqrt(pow(this->x, 2.0f) + pow(this->y, 2.0f));
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Perpendicular() const
	{
		return Vector2F(-this->y, this->x);
	}

	//----------------------------------------------------------------------------------------
	std::string Vector2F::ToMString() const
	{
		std::string vectorDisplayed;
		vectorDisplayed.append("x ->");
		vectorDisplayed.append(std::to_string(this->x));
		vectorDisplayed += ", y -> ";
		vectorDisplayed += std::to_string(this->y);
		return vectorDisplayed;
	}

	//----------------------------------------------------------------------------------------
	float Vector2F::Magnitude(Vector2F const& v1, Vector2F const& v2)
	{
		return sqrt(pow((v2.x - v1.x), 2.0f) + pow((v2.y - v1.y), 2.0f));
	}

	//----------------------------------------------------------------------------------------
	bool Vector2F::AreSimilar(Vector2F const& v1, Vector2F const& v2, float epsilon)
	{
		return Magnitude(v1, v2) < epsilon;
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Abs(Vector2F const & v)
	{
		return Vector2F(fabs(v.x), fabs(v.y));
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Mid(Vector2F const& v1, Vector2F const& v2)
	{
		return Vector2F(float((v1.x + v2.x) * 0.5), float((v1.y + v2.y) * 0.5));
	}

	Vector2F Vector2F::Left = Vector2F(-1.f, 0.f);
	Vector2F Vector2F::Right = Vector2F(1.f, 0.f);
	Vector2F Vector2F::Up = Vector2F(0.f, 1.f);
	Vector2F Vector2F::Down = Vector2F(0.f, -1.f);
	Vector2F Vector2F::Zero = Vector2F(0.f, 0.f);
}
#pragma endregion

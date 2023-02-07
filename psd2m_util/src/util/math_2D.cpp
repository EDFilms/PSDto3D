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
	std::string Vector2F::ToMString() const
	{
		std::string vectorDisplayed;
		vectorDisplayed.append("x ->");
		vectorDisplayed.append(std::to_string(this->x));
		vectorDisplayed += ", y -> ";
		vectorDisplayed += std::to_string(this->y);
		return vectorDisplayed;
	}

	Vector2F Vector2F::Left = Vector2F(-1.f, 0.f);
	Vector2F Vector2F::Right = Vector2F(1.f, 0.f);
	Vector2F Vector2F::Up = Vector2F(0.f, 1.f);
	Vector2F Vector2F::Down = Vector2F(0.f, -1.f);
	Vector2F Vector2F::Zero = Vector2F(0.f, 0.f);

}
#pragma endregion

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

#ifndef VECTORIELPATH_H
#define VECTORIELPATH_H

#include <vector>
#include "math_2D.h"

namespace util
{
	//----------------------------------------------------------------------------------------------
	struct PathPoints
	{
		bool IsLinked = false;
		Vector2F AnchorPoint;
		Vector2F SegIn;
		Vector2F SegOut;

		PathPoints(){};
		~PathPoints() {};
	};

	//----------------------------------------------------------------------------------------------
	struct PathRecord
	{
		bool IsClosedPath = false;
		std::vector<PathPoints*> Points;

		PathRecord() {};
		~PathRecord() {};
	};
}

#endif // VECTORIELPATH_H

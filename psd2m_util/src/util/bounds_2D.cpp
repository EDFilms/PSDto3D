//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file BoundingBox.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "helpers.h"
#include "bounds_2D.h"
#include <iostream>
#include <string>
#include <algorithm>


namespace util
{

#pragma region TRANSFORM

	//----------------------------------------------------------------------------------------
	void xformUV::InitTransform(const boundsUV& boundsFrom, const boundsUV& boundsTo, bool rotate)
    {
		float rotation;
		Vector2F scale;
		Vector2F translation;

		// Calculate scale and rotation components of 2D transform
		Vector2F spanFrom = boundsFrom.GetSize(), spanTo = boundsTo.GetSize();
		if( rotate )
		{
			rotation = -90.0f;
			scale.x = spanTo.y / spanFrom.x;
			scale.y = spanTo.x / spanFrom.y;
		}
		else
		{
			rotation = 0.0f;
			scale.x = spanTo.x / spanFrom.x;
			scale.y = spanTo.y / spanFrom.y;
		}
		Matrix2F m = Matrix2F::Rotation(rotation).MultiplyRight( Matrix2F::Scale( scale.x, scale.y ) );

		// Calculate transform component of 2D transform
		Vector2F p = m.Transform( boundsFrom.BottomLeftPoint() );
		if( rotate )
		{
			p = boundsTo.TopLeftPoint() - p;
		}
		else
		{
			p = boundsTo.BottomLeftPoint() - p;
		}
		m.tx = p.x;
		m.ty = p.y;

		// Assign the transform matrix
		this->mat = m;


		// Here's an example of a texture rotation, with upper left corner going from (0,0.5) -> (0.75,0),
		// source width is 1.0 and height 0.25, dest width is 0.25 and height 1.0

		// FROM
		//(0,0)                 (0,0)
		//[ . . . . . . . .]    [ . . . . . . # D]
		//[ . . . . . . . .]    [ . . . . . . # L]
		//[ . . . . . . . .]    [ . . . . . . # R]
		//[ . . . . . . . .]    [ . . . . . . O O]
		//[ H E L L O # # #]    [ . . . . . . L W]
		//[ # # # W O R L D]    [ . . . . . . L #]
		//[ . . . . . . . .]    [ . . . . . . E #]
		//[ . . . . . . . .]    [ . . . . . . H #]
		//             (1,1)                 (1,1)

		//// To perform UV transformation on a point v:

		//// rotated case
		////   dst.x =  ((v.y - srcMin.y)/srcHeight)*dstWidth + dstMin.x
		////         =  v.y * S.y + T.y  ...   S.y = dstWidth/srcHeight, T.y = dstMin.x - srcMin.y * S.y
		////   dst.y =  (1.0 - ((v.x - srcMin.x)/srcWidth))*dstHeight + dstMin.y
		////         =  v.x * S.x + dstHeight + dstMin.y - srcMin.x * S.x  ...  S.x = -dstHeight/srcWidth
		////         =  v.x * S.x + T.x  ...  S.x = -dstHeight/srcWidth, T.x = dstHeight + dstMin.y - srcMin.x * S.x

		//// non-rotated case
		////   dst.x = (( (v.x - srcMin.x) / srcWidth ) * dstWidth) + dstMin.x ...
		////         = (v.x * S.x)  +  (dstMin.x - (srcMin.x * S.x)) ... S.x = dstWidth / srcWidth
		////   dst.y = (v.y * S.y)  +  (dstMin.y - (srcMin.y * S.y)) ... S.y = dstHeight / srcHeight

		//Vector2F srcMin = boundsFrom.min_point, dstMin = boundsTo.min_point;
		//Vector2F srcSize = boundsFrom.GetSize(), dstSize = boundsTo.GetSize();
		//float srcWidth = srcSize.x, srcHeight = srcSize.y, dstWidth = dstSize.x, dstHeight = dstSize.y;
		//this->rotate = (rotate? 1:0);
		//if( this->rotate )
		//{
		//	this->scale = Vector2F( -dstHeight/srcWidth, dstWidth/srcHeight ); // -1.0, 1.0
		//	this->translate = Vector2F(
		//		dstHeight + dstMin.y - (srcMin.x * scale.x), // 1.0
		//		dstMin.x - (srcMin.y * scale.y) ); // 0.25
		//	// Transform() -> return Vector2F( translate.y + (v.y*scale.y), translate.x + (v.x*scale.x) )
		//	// [0.0,0.50] -> [0.75, 1.00] 
		//	// [1.0,0.75] -> [1.00, 0.00]
		//}
		//else
		//{
		//	// scalar is (layerSize/atlasSize) and offset is atlasPos+(-scalar*(layerPos+atlasPos)) ...
		//	this->scale = Vector2F( dstWidth/srcWidth, dstHeight/srcHeight );
		//	this->translate = Vector2F(
		//		dstMin.x + (-scale.x*(srcMin.x)),
		//		dstMin.y + (-scale.y*(srcMin.y)) );
		//}
    }

	//----------------------------------------------------------------------------------------
	void xformUV::InvertTransform()
	{
		mat.Invert();

		// TODO: fails with rotate=true, transform won't produce meaning results after inversion

		//// Example, scale [2.0,2.0] transform [+1.0,+1.0] -> scale [0.5,0.5] transform [-0.5,-0.5]
		//translate.x = -translate.x/scale.x;
		//translate.y = -translate.y/scale.y;
		//scale.x = 1.0f/scale.x;
		//scale.y = 1.0f/scale.y;
	}

#pragma endregion

#pragma region CALCUL BOUNDS

	//----------------------------------------------------------------------------------------
	void boundsUV::Init( const boundsPixels& inset, int imageWidth, int imageHeight )
	{
		boundsPixels boundsFrame( 0,0, imageWidth,imageHeight );
		GenerateBoundingBox( inset, boundsFrame );
	}

	//----------------------------------------------------------------------------------------
	void boundsUV::GenerateBoundingBox(float u, float v, float width, float height)
	{
		this->min_point = Vector2F( u, v );
		this->max_point = Vector2F( u+width, v+height );
	}

	void boundsUV::GenerateBoundingBox(const boundsPixels& inset, const boundsPixels& frame)
	{
		float uOffset = inset.XPixels() / (float)(frame.WidthPixels());
		float vOffset = inset.YPixels() / (float)(frame.HeightPixels());
		float uScale  = inset.WidthPixels() / (float)(frame.WidthPixels());
		float vScale  = inset.HeightPixels() / (float)(frame.HeightPixels());
		GenerateBoundingBox( uOffset, vOffset, uScale, vScale );
	}

	//----------------------------------------------------------------------------------------
	void boundsUV::GenerateBoundingBox(std::vector<Vector2F*> const& pathPoints)
	{
		Vector2F min_point = Vector2F( 2.0f,  2.0f);
		Vector2F max_point = Vector2F(-1.0f, -1.0f);

		for (int i = -0; i < pathPoints.size(); i++)
		{
			min_point.x = std::min(min_point.x, pathPoints[i]->x);
			min_point.y = std::min(min_point.y, pathPoints[i]->y);
			max_point.x = std::max(max_point.x, pathPoints[i]->x);
			max_point.y = std::max(max_point.y, pathPoints[i]->y);
		}

		this->min_point = min_point;
		this->max_point = max_point;
	}


	//----------------------------------------------------------------------------------------
	void boundsUV::DisplayBoundingBox() const
	{
		std::string boxValue = "BoundingBox\n";
		boxValue.append("Min Point: ");
		boxValue.append(this->min_point.ToMString());
		boxValue.append(";\nMax Point: ");
		boxValue.append(this->max_point.ToMString());
		std::cout << boxValue << std::endl;;
	}

#pragma endregion

#pragma region ORIENTED BOUNDING BOX FROM OPEN VECTOR

	//----------------------------------------------------------------------------------------------
	void boundsUV::Expand(boundsUV bounds)
	{
		this->min_point.x = MIN( this->min_point.x, bounds.min_point.x );
		this->min_point.y = MIN( this->min_point.y, bounds.min_point.y );
		this->max_point.x = MAX( this->max_point.x, bounds.max_point.x );
		this->max_point.y = MAX( this->max_point.y, bounds.max_point.y );
	}

	//----------------------------------------------------------------------------------------------
	void boundsUV::Expand( int imageWidth, int imageHeight, int paddingPixels )
	{
		boundsPixels boundsFrame = boundsPixels( 0, 0, imageWidth, imageHeight );
		boundsPixels boundsRegion; // pixel bounds of mesh
		boundsRegion.Init( *this,  imageWidth, imageHeight );
		boundsPixels boundsPadded = boundsPixels(
			boundsRegion.XPixels()-paddingPixels, boundsRegion.YPixels()-paddingPixels,
			boundsRegion.WidthPixels()+(2*paddingPixels), boundsRegion.HeightPixels()+(2*paddingPixels) );
		GenerateBoundingBox( boundsPadded, boundsFrame );
	}

	//----------------------------------------------------------------------------------------------
	void boundsUV::Clip(boundsUV bounds)
	{
		this->min_point.x = MAX( this->min_point.x, bounds.min_point.x );
		this->min_point.y = MAX( this->min_point.y, bounds.min_point.y );
		this->max_point.x = MIN( this->max_point.x, bounds.max_point.x );
		this->max_point.y = MIN( this->max_point.y, bounds.max_point.y );
	}


#pragma endregion

#pragma region ORIENTED BOUNDING BOX

	//----------------------------------------------------------------------------------------
	double boundsUV::Cross(const Vector2F &o, const Vector2F &a, const Vector2F &b)
	{
		return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
	}

	//----------------------------------------------------------------------------------------
	std::vector<Vector2F> boundsUV::ConvexHull(std::vector<Vector2F>& points)
	{
		int n = int(points.size());
		int k = 0;
		std::vector<Vector2F> H(2 * n);

		// Sort Points lexicographically
		sort(points.begin(), points.end());

		// Build lower hull
		for (int i = 0; i < n; ++i) {
			while (k >= 2 && Cross(H[k - 2], H[k - 1], points[i]) <= 0) k--;
			H[k++] = points[i];
		}

		// Build upper hull
		for (int i = n - 2, t = k + 1; i >= 0; i--) {
			while (k >= t && Cross(H[k - 2], H[k - 1], points[i]) <= 0) k--;
			H[k++] = points[i];
		}

		H.resize(k - 1);
		return H;
	}

#pragma endregion

#pragma region BOUNDING RECT

	//----------------------------------------------------------------------------------------
	boundsPixels::boundsPixels( int x, int y, int width, int height )
	{
		this->X = x;
		this->Y = y;
		this->Width = width;
		this->Height = height;
	}

	//----------------------------------------------------------------------------------------
	void boundsPixels::Init( const boundsUV& bounds, int imageWidth, int imageHeight )
	{
		int left = (int)floor( bounds.TopLeftPoint().x * imageWidth );
		int right = (int)ceil( bounds.BottomRightPoint().x * imageWidth );
		// Bounds in UV space are inverted vertically from pixel space,
		// TopLeft is actually BottomLeft and BottomRight is actually TopRight
		int bottom = (int)floor( bounds.TopLeftPoint().y * imageHeight );
		int top = (int)ceil( bounds.BottomRightPoint().y * imageHeight );
		this->X = left;
		this->Y = top;
		this->Width = abs(right-left);
		this->Height = abs(bottom-top);
	}

	//----------------------------------------------------------------------------------------------
	void boundsPixels::Expand( int paddingPixels )
	{
		this->X -= paddingPixels;
		this->Y -= paddingPixels;
		this->Width += (2*paddingPixels);
		this->Height += (2*paddingPixels);
	}

	//----------------------------------------------------------------------------------------
	void boundsPixels::Clip( int imageWidth, int imageHeight )
	{
		// slide the rect up, if below the minimums
		int underscanX = (-this->X);					// positive if underscan
		int underscanY = (-this->Y);
		this->X += MAX(0, underscanX);					// if no underscan, do nothing
		this->Width -= MAX(0, underscanX);
		this->Y += MAX(0, underscanY);
		this->Height -= MAX(0, underscanY);
		// slide the rect down, if above the maximums
		int overscanX = (this->X - imageWidth);			// positive if overscan
		int overscanY = (this->Y - imageHeight);
		this->X -= MAX(0, overscanX);					// if no overscan, do nothing
		this->Width += MAX(0, overscanX);
		this->Y -= MAX(0, overscanY);
		this->Height += MAX(0, overscanY);
		// resize the rect up, if if below the minimum
		this->Width = MAX(0, this->Width);
		this->Height = MAX(0, this->Height);
		// resize the rect down, if above the maximums
		this->Width = MIN((imageWidth-this->X), this->Width);
		this->Height = MIN((imageHeight-this->Y), this->Height);
	}

	//----------------------------------------------------------------------------------------
	bool boundsPixels::Contains( int x, int y ) const
	{
		if( (x<this->X) || (y<this->Y) || (x>=(this->X + this->Width)) || (y>=(this->Y + this->Height)) )
			return false;
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool boundsPixels::IsRotatedRelativeTo( boundsPixels bounds )
	{
		float this_aspect = 2.0f;
		float that_aspect = 2.0f;
		if( this->Height>0 ) this_aspect = (float)this->Width / (float)this->Height;
		if( bounds.Height>0 ) that_aspect = (float)bounds.Width / (float)bounds.Height;
		return ( ((this_aspect>1.0f) && (that_aspect<1.0f)) || ((this_aspect<1.0f) && (that_aspect>1.0f)) );
	}


#pragma endregion

}

//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file BoundingBox.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "util/math_2D.h"
#include <vector>

namespace util
{
	class boundsUV;
	class boundsPixels;

	//----------------------------------------------------------------------------------------------
	// Bounding box measured in UV space coordinates, range [0-1]
	class boundsUV
	{
	public:
		inline boundsUV() : min_point(1,1), max_point(-1,-1) {}
		inline boundsUV( const boundsUV& that) = default;
		inline boundsUV( float u, float v, float width, float height ) { GenerateBoundingBox(u,v,width,height); }
		void Init( const boundsPixels& bounds, int imageWidth, int imageHeight );

		inline Vector2F TopLeftPoint() const		{ return Vector2F( min_point.x, max_point.y ); }
		inline Vector2F TopRightPoint() const		{ return Vector2F( max_point.x, max_point.y ); }
		inline Vector2F BottomRightPoint() const	{ return Vector2F( max_point.x, min_point.y ); }
		inline Vector2F BottomLeftPoint() const		{ return Vector2F( min_point.x, min_point.y ); }
		inline Vector2F MinPoint() const			{ return min_point; } // bottom left is min
		inline Vector2F MaxPoint() const			{ return max_point; } // top right is max
		inline Vector2F GetCenter() const			{ return Vector2F::Mid( min_point, max_point ); }
		inline Vector2F GetSize() const				{ return Vector2F::Span( min_point, max_point ); }

		void GenerateBoundingBox(float u, float v, float width, float height);
		void GenerateBoundingBox(const boundsPixels& inset, const boundsPixels& frame);
		void GenerateBoundingBox(std::vector<Vector2F*> const& pathPoints); // TODO: array of pointers has caused bugs
		void DisplayBoundingBox() const;
		void Expand(boundsUV bounds);
		void Expand(int imageWidth, int imageHeight, int paddingPixels); // unclipped
		void Clip(boundsUV bounds = boundsUV(0,0,1,1)); // clip to range, [0,0]-[1,1] by default

	private:
        friend class xformUV;
		Vector2F min_point, max_point;

		static double Cross(Vector2F const& o, Vector2F const& a, Vector2F const& b);
		std::vector<Vector2F> ConvexHull(std::vector<Vector2F>& points);
	};

	//----------------------------------------------------------------------------------------------
	// Bounding box measured in pixels, range [0-imageSize]
	class boundsPixels
	{
	public:
		inline boundsPixels() : X(0), Y(0), Width(0), Height(0) {};
		inline boundsPixels( const boundsPixels& that ) = default;
		boundsPixels( int x, int y, int width, int height );
		void Init( const boundsUV& bounds, int imageWidth, int imageHeight );

		inline int TopPixel() const		{ return Y; }
		inline int LeftPixel() const	{ return X; }
		inline int BottomPixel() const	{ return Y+Height; } // actually one past bottom-most pixel
		inline int RightPixel() const	{ return X+Width; }  // actually one past right-most pixel
		inline int XPixels() const		{ return X; }
		inline int YPixels() const		{ return Y; }
		inline int WidthPixels() const	{ return Width; }
		inline int HeightPixels() const	{ return Height; }
		void Expand(int paddingPixels); // unclipped
		void Clip( int imageWidth, int imageHeight ); // clip to [0,0]-[width,height] range
		bool Contains( int x, int y ) const;
		bool IsRotatedRelativeTo( boundsPixels bounds );

	private:
		int X;
		int Y;
		int Width;
		int Height;
	};


	//----------------------------------------------------------------------------------------------
	// Tranform from one UV coordinate space to another; from original bitmap to atlas region
	class xformUV
	{
	public:
		//inline xformUV() : translate(0.0f,0.0f), scale(1.0f,1.0f), rotate(0) {}
		inline xformUV() {}
		void InitTransform(const boundsUV& boundsFrom, const boundsUV& boundsTo, bool rotate=false);

		void InvertTransform();
		Vector2F GetScale() const;
		float GetRotation() const;
		inline Vector2F GetTranslate() const { return Vector2F( mat.tx, mat.ty ); }
		inline void GetRotationAndScale( float& angle, float& scaleX, float& scaleY )
		{ return mat.GetRotationAndScale( angle, scaleX, scaleY ); }

		inline Vector2F Transform(const Vector2F& v) const { return mat.Transform(v); }
		//inline Vector2F Transform(const Vector2F& v) const
		//{
		//	if( rotate )
		//		return Vector2F( translate.y + (v.y*scale.y), translate.x + (v.x*scale.x) );
		//	return Vector2F( translate.x + (v.x*scale.x), translate.y + (v.y*scale.y) );
		//}
	private:
		Matrix2F mat;
		//Vector2F translate,scale;
		//int rotate;
	};
}
#endif // BOUNDINGBOX_H

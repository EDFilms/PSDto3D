//----------------------------------------------------------------------------------------------
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file bitmap.h
//  @author Michaelson Britt
//  @date 02-01-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef BITMAP_H
#define BITMAP_H

#include "util/helpers.h"

namespace util
{
	//----------------------------------------------------------------------------------------
	template<typename T>
	class Bitmap
	{
	public:
		std::vector<T> data;
		int width, height;
		Bitmap() : width(0), height(0) {}
		inline void Init( int width, int height, T& val )
		{
			this->width = width, this->height = height;
			data.resize( width*height, val );
		}
		inline bool IsValid( int x, int y ) { return ((x>=0) && (x<width) && (y>=0) && (y<height)); }
		inline T& Get( int x, int y ) { return data[ (y*width) + x ]; }
		inline void Set( int x, int y, T& val ) { if( IsValid(x,y) ) data[ (y*width) + x ] = val; }
		inline void DrawLine( T& val, int x0, int y0, int x1, int y1, bool fill_corners )
		{
			bool horz = (MAX(x0,x1)-MIN(x0,x1)) > (MAX(y0,y1)-MIN(y0,y1));
			if( horz ) DrawLineHorz( val, x0, y0, x1, y1, fill_corners );
			else       DrawLineVert( val, x0, y0, x1, y1, fill_corners );
		}
		typedef int (*CheckLineFn)(void* param, int x, int y);
		inline void CheckLine( CheckLineFn func, void* param, int x0, int y0, int x1, int y1 )
		{
			bool horz = (MAX(x0,x1)-MIN(x0,x1)) > (MAX(y0,y1)-MIN(y0,y1));
			if( horz ) CheckLineHorz( func, param, x0, y0, x1, y1 );
			else       CheckLineVert( func, param, x0, y0, x1, y1 );
		}

	protected:
		// assumes line is more horizontal than vertical, no antialiasing but can fill corner pixels after step
		inline void DrawLineHorz( T& val, int x0, int y0, int x1, int y1, bool fill_corners )
		{
			if( x0>x1 ) { Swap(x0,x1); Swap(y0,y1); } // ensure point x0,y0 has the lower x value
			int x_span = (x1-x0);
			if( x_span==0 ) x_span=1; // avoids divide by zero, but not strictly necessary
			float yf = y0+0.5f;
			float yf_inc = (y1-y0)/(float)x_span;
			int y_prev = y0;
			for( int x=x0; true; x++, yf+=yf_inc )
			{
				int y = (int)yf;
				Set( x, y, val );
				if( fill_corners && (y!=y_prev) ) Set( x, y_prev, val );
				y_prev = y;
				if( x==x1 ) break;
			}
		}

		// assumes line is more vertical than horizontal, no antialiasing but can fill corner pixels after step
		inline void DrawLineVert( T& val, int x0, int y0, int x1, int y1, bool fill_corners )
		{
			if( y0>y1 ) { Swap(x0,x1); Swap(y0,y1); } // ensure point x0,y0 has the lower y value
			int y_span = (y1-y0);
			if( y_span==0 ) y_span=1; // avoids divide by zero, but not strictly necessary
			float xf = x0+0.5f;
			float xf_inc = (x1-x0)/(float)y_span;
			int x_prev = x0;
			for( int y=y0; true; y++, xf+=xf_inc )
			{
				int x = (int)xf;
				Set( x, y, val );
				if( fill_corners && (x!=x_prev) ) Set( x_prev, y, val );
				x_prev = x;
				if( y==y1 ) break;
			}
		}

		// assumes line is more horizontal than vertical, calls function at each pixel
		inline void CheckLineHorz( CheckLineFn func, void* param, int x0, int y0, int x1, int y1 )
		{
			if( x0>x1 ) { Swap(x0,x1); Swap(y0,y1); } // ensure point x0,y0 has the lower x value
			int x_span = (x1-x0);
			if( x_span==0 ) x_span=1; // avoids divide by zero, but not strictly necessary
			float yf = y0+0.5f;
			float yf_inc = (y1-y0)/(float)x_span;
			int y_prev = y0;
			for( int x=x0; true; x++, yf+=yf_inc )
			{
				int y = (int)yf;
				int result = func( param, x, y );
				if( result<0 ) break;
				if( x==x1 ) break;
			}
		}

		// assumes line is more vertical than horizontal, calls function at each pixel
		inline void CheckLineVert( CheckLineFn func, void* param, int x0, int y0, int x1, int y1 )
		{
			if( y0>y1 ) { Swap(x0,x1); Swap(y0,y1); } // ensure point x0,y0 has the lower y value
			int y_span = (y1-y0);
			if( y_span==0 ) y_span=1; // avoids divide by zero, but not strictly necessary
			float xf = x0+0.5f;
			float xf_inc = (x1-x0)/(float)y_span;
			int x_prev = x0;
			for( int y=y0; true; y++, xf+=xf_inc )
			{
				int x = (int)xf;
				int result = func( param, x, y );
				if( result<0 ) break;
				if( y==y1 ) break;
			}
		}
	};
}

#endif // BITMAP_H
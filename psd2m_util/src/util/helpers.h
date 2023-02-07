//----------------------------------------------------------------------------------------------
//
//  @file Helpers.h
//  @author Michaelson Britt
//  @date 08-21-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef HELPERS_H
#define HELPERS_H

#include <math.h>

namespace util {
	void DebugPrint(const char* lpszFormat, ...); // forward declaration
};

//----------------------------------------------------------------------------------------
// Math helpers

#define MIN(a,b) ((a)<=(b)? (a):(b))
#define MAX(a,b) ((a)>=(b)? (a):(b))
#define CLAMP(x,lo,hi) ((x)>(hi)?  (hi) : ((x)<(lo)?  (lo) : (x)) )

inline float Lerp( float a, float b, float w )
{
	return ((1.0f-w)*a + (w*b));
}

inline double Lerp( double a, double b, double w )
{
	return ((1.0f-w)*a + (w*b));
}

inline int Lerp( int a, int b, float w )
{
	return (int)((1.0f-w)*a + (w*b));
}

inline void Swap( int& a, int& b)
{
	int temp = a;
	a = b;
	b = temp;
}


//----------------------------------------------------------------------------------------
// Memory validation helpers

// pass the address of a std::vector<>, checks for memory corruption
inline bool CheckVector( void* vector, int size )
{
	if( size >= 4096 )
	{
		intptr_t check1 = *(intptr_t*)vector;
		intptr_t check2 = *(intptr_t*)(check1-8);
		if( check1 <= check2 ) // (check1-check2)!=16
		{
			util::DebugPrint("Error: memory corruption in std::vector(), at %llX, check1=%llX, check2=%llX  \n",
				(intptr_t)vector, check1, check2 );
			return false; // error
		}
	}
	return true; // default, check pass
}

//----------------------------------------------------------------------------------------
// Interpolation helpers

// Interpolate along an expotential curve, based on desired outputs for low, mid and high
// That is, based on desired output values when input values are 0.0, 0.5 and 1.0
// For example, if low=3.0, mid=3.25, high=4.0, then the curve is shaped like y=x^2,
// or specifically, output=(input^2)+3.0 assuming input is in range [0-1]
class CurveInterp
{
    public:
        CurveInterp()
		: lo(0), mid(0.5f), hi(1), offset(0), scale(1), exp(1),
		  lo_linear(false), hi_linear(false) {}
        ~CurveInterp() {}
        inline void Init( float lo_in, float mid_in, float hi_in, bool lo_linear_in, bool hi_linear_in )
        {
			this->lo = lo_in;
			this->mid = mid_in;
			this->hi = hi_in;
            this->offset = lo_in;
            this->scale = (hi_in - lo_in);
			this->lo_linear = lo_linear_in;
			this->hi_linear = hi_linear_in;
            
            float s = ((mid_in - lo_in) / (hi_in - lo_in));
            //solve for exp in equation of (0.5 ^ exp) = s ...
            // ... (2 ^ -1) ^ exp = s
            // ... (2 ^ -exp) = s
            // ... log2( 2 ^ -exp ) = log2( s )
            // ... (-exp) = log2( s )
            //     exp = -log2(s)
            this->exp = (float)(-log2(s));
        }
        inline float Interp( float w ) const // input must be in range [0-1]
        {
			// optional linear interpolation for lower or upper half of range
			if( lo_linear && (w<0.5f) )
				return Lerp(lo,mid, 2.0f*w);
			if( hi_linear && (w>0.5f) )
				return Lerp(mid,hi,(w-0.5f)*2.0f);
			// else curve interpolation
            float p = (float)(pow( w, exp ));
            return ((p*scale) + offset);
        }
        float lo, mid, hi;
        float offset, scale, exp;
		// options to convert low or high end of the curve into flat linear interpolation instead
		bool lo_linear, hi_linear;
};



//----------------------------------------------------------------------------------------
// Iteration helpers

// USAGE:
//    for (auto& item : vec)
//    {
//        int index = iter_index(item,vec);
//    }
// IMPORTANT: Only works with by-reference auto& and not with by-value auto
template <class T, class E>
int iter_index( const E& item, const T& vec )
{
	intptr_t mem_dist = ((intptr_t)(&item)) - ((intptr_t)(&(vec[0])));
	return (int)(mem_dist/sizeof(vec[0]));
}



#endif // HELPERS_H

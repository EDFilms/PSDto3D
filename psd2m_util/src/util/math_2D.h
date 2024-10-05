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

#ifndef   PI
#define   PI      3.1415926535897932384626433832795
#endif // PI
#ifndef   TWO_PI
#define   TWO_PI  6.283185307179586476925286766559
#endif // TWO_PI


namespace util
{
	//----------------------------------------------------------------------------------------
	struct Vector2F
	{
		float x;
		float y;

		inline Vector2F();
		inline Vector2F(float x, float y);
		inline Vector2F(Vector2F const& v);
		inline ~Vector2F();

		inline bool operator==(const Vector2F& a) const;
		inline bool operator!=(const Vector2F& a) const;
		inline bool operator< (const Vector2F& rhs) const;
		inline float operator*(const Vector2F& a) const;
		inline Vector2F operator-() const;
		inline Vector2F operator+(const Vector2F& a) const;
		inline Vector2F operator-(const Vector2F& a) const;
		inline Vector2F operator*(const float& n) const;
		inline Vector2F operator/(const float& n) const;
		inline Vector2F& operator+=(const Vector2F& a);
		inline Vector2F& operator-=(const Vector2F& a);
		inline Vector2F& operator*=(const float& n);
		inline Vector2F& operator/=(const float& n);

		inline float Magnitude() const;
		inline Vector2F Perpendicular() const;

		std::string ToMString() const;
		inline static float Magnitude(Vector2F const& v1, Vector2F const& v2);
		inline static bool AreSimilar(Vector2F const& v1, Vector2F const& v2, float epsilon = 0.001f);
		inline static Vector2F Abs(Vector2F const& v);
		inline static Vector2F Mid(Vector2F const & v1, Vector2F const & v2);
		inline static Vector2F Span(Vector2F const & v1, Vector2F const & v2);

		static Vector2F Left;
		static Vector2F Right;
		static Vector2F Up;
		static Vector2F Down;
		static Vector2F Zero;
	};

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
	bool Vector2F::operator!=(const Vector2F& a) const
	{
		return (!(operator==(a)));
	}

	//----------------------------------------------------------------------------------------
	bool Vector2F::operator<(const Vector2F & rhs) const
	{
		if( this->x == rhs.x )
			return (this->y < rhs.y);
		return (this->x < rhs.x);
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::operator-() const
	{
		return Vector2F(-(this->x), -(this->y));
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
		return (float)(sqrt(pow(this->x, 2.0f) + pow(this->y, 2.0f)));
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Perpendicular() const
	{
		return Vector2F(-this->y, this->x);
	}

	//----------------------------------------------------------------------------------------
	float Vector2F::Magnitude(Vector2F const& v1, Vector2F const& v2)
	{
		return (float)(sqrt(pow((v2.x - v1.x), 2.0f) + pow((v2.y - v1.y), 2.0f)));
	}

	//----------------------------------------------------------------------------------------
	bool Vector2F::AreSimilar(Vector2F const& v1, Vector2F const& v2, float epsilon)
	{
		return Magnitude(v1, v2) < epsilon;
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Abs(Vector2F const & v)
	{
		return Vector2F((float)fabs(v.x), (float)fabs(v.y));
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Mid(Vector2F const& v1, Vector2F const& v2)
	{
		return Vector2F(float((v1.x + v2.x) * 0.5), float((v1.y + v2.y) * 0.5));
	}

	//----------------------------------------------------------------------------------------
	Vector2F Vector2F::Span(Vector2F const& v1, Vector2F const& v2)
	{
		return Vector2F( (float)fabs(v2.x - v1.x), (float)fabs(v2.y - v1.y) );
	}

	// TODO: These should be members of Vector2F
	inline float DotProduct( const Vector2F& a, const Vector2F& b )
	{
	  return ( (a.x * b.x) + (a.y * b.y) );
	}

	inline float Length( const Vector2F& p )
	{
		return (float)(sqrt( (p.x*p.x) + (p.y*p.y) ));
	}

	inline float Dist( const Vector2F& a, const Vector2F& b )
	{
		float x = b.x-a.x, y = b.y-a.y;
		return (float)(sqrt( (x*x) + (y*y) ));
	}

	inline Vector2F Normalize( const Vector2F& vec )
	{
		float length = Length(vec);
		if( length<=0.000001f )
			return Vector2F(1.0f,0.0f); // TODO: Error handling
		return Vector2F( vec.x/length, vec.y/length );
	}

	//----------------------------------------------------------------------------------------
	struct Vector2D
	{
		double x;
		double y;

		inline Vector2D();
		inline Vector2D(double x, double y);
		inline Vector2D(Vector2D const& v);
		inline Vector2D(Vector2F const& v);
		inline ~Vector2D();

		inline Vector2D operator+(const Vector2D& a) const;
	};

	//----------------------------------------------------------------------------------------
	Vector2D::Vector2D()
	{
		this->x = 0.0f;
		this->y = 0.0f;
	}

	//----------------------------------------------------------------------------------------
	Vector2D::Vector2D(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	//----------------------------------------------------------------------------------------
	Vector2D::Vector2D(Vector2D const& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	//----------------------------------------------------------------------------------------
	Vector2D::Vector2D(Vector2F const& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	//----------------------------------------------------------------------------------------
	Vector2D::~Vector2D() = default;

	//----------------------------------------------------------------------------------------
	Vector2D Vector2D::operator+(const Vector2D& a) const
	{
		return Vector2D(this->x + a.x, this->y + a.y);
	}

	inline double DotProduct( const Vector2D& a, const Vector2D& b )
	{
	  return ( (a.x * b.x) + (a.y * b.y) );
	}

	inline double Length( const Vector2D& p )
	{
		return sqrt( (p.x*p.x) + (p.y*p.y) );
	}

	inline double Dist( const Vector2D& a, const Vector2D& b )
	{
		double x = b.x-a.x, y = b.y-a.y;
		return sqrt( (x*x) + (y*y) );
	}

	inline Vector2D Normalize( const Vector2D& vec )
	{
		double length = Length(vec);
		if( length<=0.000001 )
			return Vector2D(1.0,0.0); // TODO: Error handling
		return Vector2D( vec.x/length, vec.y/length );
	}

	inline Vector2F Lerp( const Vector2F& point_from, const Vector2F& point_to, float w )
	{
		return Vector2F( ((1.0f-w)*point_from.x) + (w*point_to.x), ((1.0f-w)*point_from.y) + (w*point_to.y) ); 
	}

	inline Vector2D Lerp( const Vector2D& point_from, const Vector2D& point_to, double w )
	{
		return Vector2D( ((1.0-w)*point_from.x) + (w*point_to.x), ((1.0-w)*point_from.y) + (w*point_to.y) ); 
	}


	//----------------------------------------------------------------------------------------
	// Affine 2D matrix with rotation, scale and translation
	struct Matrix2F
	{
		// [ a b tx ]  ->  Multiply by [vx vy] is  [ a b tx ] [ vx ] = [ (vx a) + (vy b) + tx ]
		// [ c d ty ]      (append 1 implictly)    [ c d ty ] [ vy ]   [ (vx c) + (vy d) + ty ]
		//                                                    [ 1  ]
		float a,b,c,d; // rotation and scale
		float tx,ty; // translation

		Matrix2F()
			: a(1), b(0), c(0), d(1), tx(0), ty(0) {}
		Matrix2F( float a, float b, float c, float d, float tx, float ty )
			: a(a), b(b), c(c), d(d), tx(tx), ty(ty) {}
		Matrix2F( const Matrix2F& that )
			: a(that.a), b(that.b), c(that.c), d(that.d), tx(that.tx), ty(that.ty) {}

		void Init( float a_in, float b_in, float c_in, float d_in, float x_in, float y_in )
		{ this->a = a_in;  this->b = b_in;  this->c = c_in;  this->d = d_in;  this->tx = x_in;  this->ty = y_in; }

		Matrix2F& operator=( const Matrix2F& that )
		{
			Init( that.a, that.b, that.c, that.d, that.tx, that.ty );
			return (*this);
		}

		inline static Matrix2F Rotation( float degrees )
		{
			float rad = (degrees / 360.0f) * (float)TWO_PI;
			return Matrix2F( (float)cos(rad), (float)-sin(rad), (float)sin(rad), (float)cos(rad), 0.0f, 0.0f );
		}

		inline static Matrix2F Translation( float x, float y )
		{
			return Matrix2F( 1.0f, 0.0f, 0.0f, 1.0f, x, y );
		}

		inline static Matrix2F Scale( float sx, float sy )
		{
			return Matrix2F( sx, 0.0f, 0.0f, sy, 0.0f, 0.0f );
		}

		// returns angle (in degrees) and scale aplied by matrix
		// TODO: assumes no skew, not a true singular value decomposition here
		inline void GetRotationAndScale( float& angle, float& scaleX, float& scaleY )
		{
			// transform the unit-length basis vectors for each axis, then check result length and angle
			// x-axis basis vector [A B][1] = [A] ... y-axis basis vector [A B][0] = [B]
			//                     [C D][0]   [C]                         [C D][1] = [D]
			Vector2F xbase( a, c ); // x-axis basis vector transformed (without translation)
			Vector2F ybase( b, d ); // y-axis basis vector transformed (without translation)
			angle = (float)(atan2( xbase.y, xbase.x ) * (360.0f/TWO_PI)); // assume no skew
			scaleX = (float)sqrt( (xbase.x * xbase.x) + (xbase.y * xbase.y) );
			scaleY = (float)sqrt( (ybase.x * ybase.x) + (ybase.y * ybase.y) );
		}

		void Invert()
		{
			float sd = ((a*d) - (b*c));
			if( sd==0.0f )
				return; // error, degenerate matrix

			Vector2F t_old( tx, ty ); // store old translation, needed below

			// calculate 2x2 non-affine inverse for rotation and scale only
			float s = 1.0f / sd;
			Init( s*d, -s*b, -s*c, s*a, 0.0f, 0.0f );

			// if the original 2x3 affine matrix transformed [0,0] to [tx,ty], then
			// the inverted matrix should transform [tx,ty] after rotation and scale back to [0,0]
			// ... M2x3_inv * [tx_old,ty_old,1] = [0,0], desired 2x3 affine transform
			// ... M2x3_inv is M2x2_inv (rotation and scale inverse) combined with translation
			// ... let V = M2x2_inv * [tx_old,ty_old], transform old translation by new inverse
			// ... then our final translation vector [tx_new,ty_new] is [-V.x,-V.y]
			Vector2F t_new = Transform( t_old );
			tx = -t_new.x;
			ty = -t_new.y;
		}

		// multiply with this matrix on left, that matrix on right
		Matrix2F MultiplyRight( Matrix2F& that ) const
		{
			float a2 = (this->a * that.a) + (this->b * that.c);
			float b2 = (this->a * that.b) + (this->b * that.d);
			float c2 = (this->c * that.a) + (this->d * that.c);
			float d2 = (this->c * that.b) + (this->d * that.d);
			float x2 = this->tx + that.tx;
			float y2 = this->ty + that.ty;
			return Matrix2F( a2, b2, c2, d2, x2, y2 );
		}

		Vector2F Transform( const Vector2F& v ) const
		{
			float vx = (v.x * this->a) + (v.y * this->b) + this->tx;
			float vy = (v.x * this->c) + (v.y * this->d) + this->ty;
			return Vector2F( vx, vy );
		}
	};

}


#endif // MATH_2D_H
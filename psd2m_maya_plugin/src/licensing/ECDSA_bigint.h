//----------------------------------------------------------------------------------------------
//
//  @file ECDSA_bigint.cpp
//  @author Michaelson Britt
//  @date 11-05-2019
//
//  @section DESCRIPTION
//  Large integer library wrapper.  Uses the FGMP public domain library wrapped as C++
//
//----------------------------------------------------------------------------------------------

//
// If only 40-bit math was needed, could use native 64-bit integers
// But, multiplying two 40-bit values produces an 80-bit value, so it doesn't work
//
// Also, need support for "None" for compatibility with original python script
// Using int64_t doesn't easily allow for "None", but could allow for "Infinity"
// typedef int64_t bigint;
// static const bigint INFINITY = 0x7FFFFFFFFFFFFFFF;
// largest positive int 8 bytes:  8877665544332211
//

#ifndef __ECDSA_BIGINT_H__
#define __ECDSA_BIGINT_H__

extern "C" {
    #define __STDC__ 1
    #include "ECDSA_fgmp.h"
    #undef __STDC__
}
#include <stdint.h>
#include <string> // std::string

#ifndef self
#define self (*this)
#endif

struct bigint
{
    enum {
        TYPE_NORMAL=0,
        TYPE_NONE=1
    };
    int type;
    MP_INT val;

    // Read from string.  If prefixed with 0x then base 16, else base 10
    bigint( const char* s, unsigned int type=TYPE_NORMAL )
    {
        mpz_init_set_si( &(self.val), 0 );
        self = s;
        self.type = type;
    }
    bigint( int32_t val=0, unsigned int type=TYPE_NORMAL )
    {
        mpz_init_set_si( &(self.val), 0 );
        self = val;
        self.type = type;
    }
    bigint( const bigint& other )
    {
        self.type = other.type;
        mpz_init_set( &(self.val), &(other.val) );
    }
    ~bigint()
    {
        mpz_clear( &(self.val) );
    }
    inline std::string str( int base=10 ) const;
    inline bigint operator=( const char* s );
    inline bigint operator=( int32_t s );
    inline bigint operator=( const bigint& other );
};
static const bigint bigint_NONE(0,bigint::TYPE_NONE);

std::string bigint::str( int base ) const
{
    unsigned int str_len = mpz_sizeinbase( &(self.val), base );
    char* buf = new char[(2*str_len)+1];
    mpz_get_str( buf, base, &(self.val) );
    std::string retval(buf);
    delete[] buf; // TODO: avoid double allocation and copy
    return  retval;
}
bigint bigint::operator=( const char* s )
{
    self.type = TYPE_NORMAL;
    mpz_init_set_si( &(self.val), 0 );
    if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
        mpz_set_str( &(self.val), s+2, 16 );
    else mpz_set_str( &(self.val), s, 10 );
    return self;
}
bigint bigint::operator=( const int32_t right )
{
    self.type = TYPE_NORMAL;
    mpz_set_si( &(self.val), right );
    return self;
}
bigint bigint::operator=( const bigint& right )
{
    self.type = right.type;
    mpz_set( &(self.val), &(right.val) );
    return self;
}

inline bool operator==( const int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )==0 );
}
inline bool operator==( const bigint& left, const int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right )==0 );
}
inline bool operator==( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return (left.type == right.type);
    return ( mpz_cmp( &(left.val), &(right.val) )==0 );
}

inline bool operator!=( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )!=0 );
}
inline bool operator!=( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right )!=0 );
}
inline bool operator!=( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return (left.type != right.type);
    return ( mpz_cmp( &(left.val), &(right.val) )!=0 );
}

inline bool operator>( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )<0 );
}
inline bool operator>( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right)>0 );
}
inline bool operator>( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return false;
    return ( mpz_cmp( &(left.val), &(right.val) )>0 );
}

inline bool operator>=( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )<=0 );
}
inline bool operator>=( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right )>=0 );
}
inline bool operator>=( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return (left.type == right.type);
    return ( mpz_cmp( &(left.val), &(right.val) )>=0 );
}

inline bool operator<( int32_t left, const bigint& right )
{
    if( right.type==bigint::bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )>0 );
}
inline bool operator<( const bigint& left, int32_t right )
{
    if( left.type==bigint::bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right )<0 );
}
inline bool operator<( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return false;
    return ( mpz_cmp( &(left.val), &(right.val) )<0 );
}

inline bool operator<=( int32_t left, const bigint& right )
{
    if( right.type==bigint::bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(right.val), left )>=0 );
}
inline bool operator<=( const bigint& left, int32_t right )
{
    if( left.type==bigint::bigint::TYPE_NONE ) return false;
    return ( mpz_cmp_si( &(left.val), right )<=0 );
}
inline bool operator<=( const bigint& left, const bigint& right )
{
    if( (left.type==bigint::TYPE_NONE) || (right.type==bigint::TYPE_NONE) )
        return (left.type == right.type);
    return ( mpz_cmp( &(left.val), &(right.val) )<=0 );
}

inline bigint operator-( const bigint& left ) // unary
{
    if( left.type==bigint::TYPE_NONE ) return left;
    bigint retval;
    mpz_neg( &(retval.val), &(left.val) );
    return retval;
}

inline bigint operator+( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint left_(left);
    mpz_add( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator+( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_add( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator+( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE ) return right;
    if( right.type==bigint::TYPE_NONE ) return left;
    bigint retval;
    mpz_add( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator-( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint left_(left);
    mpz_sub( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator-( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_sub( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator-( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE ) return right;
    if( right.type==bigint::TYPE_NONE ) return left;
    bigint retval;
    mpz_sub( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator*( int32_t left, const bigint& right )
{
    if( right.type==bigint::bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint left_(left);
    mpz_mul( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator*( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_mul( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator*( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    mpz_mul( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator/( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint left_(left);
    mpz_div( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator/( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_div( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator/( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    mpz_div( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator%( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return right;
    bigint retval;
    bigint left_(left);
    mpz_mmod( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator%( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_mmod( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator%( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE ) return bigint_NONE;
    if( right.type==bigint::TYPE_NONE ) return right;
    bigint retval;
    mpz_mmod( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator&( int32_t left, const bigint& right )
{
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    bigint left_(left);
    mpz_and( &(retval.val), &(left_.val), &(right.val) );
    return retval;
}
inline bigint operator&( const bigint& left, int32_t right )
{
    if( left.type==bigint::TYPE_NONE )  return bigint_NONE;
    bigint retval;
    bigint right_(right);
    mpz_and( &(retval.val), &(left.val), &(right_.val) );
    return retval;
}
inline bigint operator&( const bigint& left, const bigint& right )
{
    if( left.type==bigint::TYPE_NONE )  return bigint_NONE;
    if( right.type==bigint::TYPE_NONE ) return bigint_NONE;
    bigint retval;
    mpz_and( &(retval.val), &(left.val), &(right.val) );
    return retval;
}

inline bigint operator+=( bigint& left, int32_t right )
{
    left = left + right;
    return left;
}
inline bigint operator+=( bigint& left, const bigint& right )
{
    left = left + right;
    return left;
}
inline bigint operator-=( bigint& left, int32_t right )
{
    left = left - right;
    return left;
}
inline bigint operator-=( bigint& left, const bigint& right )
{
    left = left - right;
    return left;
}
inline bigint operator*=( bigint& left, int32_t right )
{
    left = left * right;
    return left;
}
inline bigint operator*=( bigint& left, const bigint& right )
{
    left = left * right;
    return left;
}

#endif // __ECDSA_BIGINT_H__

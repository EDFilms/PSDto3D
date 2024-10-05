//---------- ----------
// Original from https://github.com/warner/python-ecdsa/blob/master/src/ecdsa/ellipticcurve.py
// Modified and ported from Python to C++ by Michaelson Britt, May 2019
//---------- ----------
//
//    """
//    Implementation of Elliptic-Curve Digital Signatures.
//    Classes and methods for elliptic-curve signatures:
//    private keys, public keys, signatures,
//    NIST prime-modulus curves with modulus lengths of
//    192, 224, 256, 384, and 521 bits.
//    Example:
//      # (In real-life applications, you would probably want to
//      # protect against defects in SystemRandom.)
//      from random import SystemRandom
//      randrange = SystemRandom().randrange
//      # Generate a public/private key pair using the NIST Curve P-192:
//      g = generator_192
//      n = g.order()
//      secret = randrange( 1, n )
//      pubkey = Public_key( g, g * secret )
//      privkey = Private_key( pubkey, secret )
//      # Signing a hash value:
//      hash = randrange( 1, n )
//      signature = privkey.sign( hash, randrange( 1, n ) )
//      # Verifying a signature for a hash value:
//      if pubkey.verifies( hash, signature ):
//        print_("Demo verification succeeded.")
//      else:
//        print_("*** Demo verification failed.")
//      # Verification fails if the hash value is modified:
//      if pubkey.verifies( hash-1, signature ):
//        print_("**** Demo verification failed to reject tampered hash.")
//      else:
//        print_("Demo verification correctly rejected tampered hash.")
//    Version of 2009.05.16.
//    Revision history:
//          2005.12.31 - Initial version.
//          2008.11.25 - Substantial revisions introducing new classes.
//          2009.05.16 - Warn against using random.randrange in real applications.
//          2009.05.17 - Use random.SystemRandom by default.
//    Written in 2005 by Peter Pearson and placed in the public domain.
//    """

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include <mbstring.h> // for _mbslwr
#include <vector>
#include "ECDSA_bigint.h"
#include "ECDSA_sha256.h"

#ifndef self
#define self (*this)
#endif

#define ASSERT(expr) if (!(expr)) aFailed(__FILE__, __LINE__)
void aFailed(const char *file, int line)
{
    printf("Assert failed: %s : %i \n", file, line);
}

// Inverse of a mod m.
bigint inverse_mod(const bigint& a_in, const bigint& m_in)
{
    bigint a = a_in;
    bigint m = m_in;
    if( (a < 0) || (m <= a) )
    {
        a = a % m;
    }

    // From Ferguson and Schneier, roughly:

    bigint c=a, d=m;
    bigint uc=1, vc=0, ud=0, vd=1;
    while( c != 0 )
    {
        bigint c_prev = c;
        bigint q = d/c;
        c = d - (q*c);
        d = c_prev;

        bigint uc_prev = uc, vc_prev = vc, ud_prev = ud, vd_prev = vd;
        uc = ud_prev - (q * uc_prev);
        vc = vd_prev - (q * vc_prev);
        ud = uc_prev;
        vd = vc_prev;
    }

    // At this point, d is the GCD, and ud*a+vd*m = d.
    // If d == 1, this means that ud is a inverse.

    //ASSERT( d == 1 );
    if( ud > 0 )
        return ud;
    else
        return ud + m;
}

// Elliptic Curve over the field of integers modulo a prime.
// The curve of points satisfying y^2 = x^3 + a*x + b (mod p).
struct CurveFp
{
    bigint __p;
    bigint __a;
    bigint __b;

    CurveFp() : __p(bigint_NONE), __a(bigint_NONE), __b(bigint_NONE) {}
    CurveFp( const bigint& p, const bigint& a, const bigint& b )
    {
        self.__p = p;
        self.__a = a;
        self.__b = b;
    }
    inline bigint p() const { return self.__p; }
    inline bigint a() const { return self.__a; }
    inline bigint b() const { return self.__b; }

    inline bool operator==( const CurveFp& other ) const;
    inline bool operator!=( const CurveFp& other ) const;
    inline bool contains_point(bigint x, bigint y) const; // Is the point (x,y) on this curve?
};
static const CurveFp CurveFp_NONE(bigint_NONE,bigint_NONE,bigint_NONE);

bool CurveFp::operator==( const CurveFp& other ) const
{
    return (self.__p==other.__p) && (self.__a==other.__a) && (self.__b==other.__b);
}
bool CurveFp::operator!=( const CurveFp& other ) const
{
    return !((self.__p==other.__p) && (self.__a==other.__a) && (self.__b==other.__b));
}
bool CurveFp::contains_point(bigint x, bigint y) const
{
    return (((y * y) - ((x * x * x) + (self.__a * x )+ self.__b)) % self.__p) == 0;
}

// A point on an elliptic curve. Altering x and y is forbidding,
// but they can be read by the x() and y() methods.
struct Point
{
    CurveFp __curve;
    bigint __x;
    bigint __y;
    bigint __order;

    Point();
    Point( const Point& other );
    Point( const CurveFp& curve, const bigint& x, const bigint& y, const bigint& order=bigint_NONE );
    inline CurveFp curve() const    { return self.__curve; }
    inline bigint x() const         { return self.__x; }
    inline bigint y() const         { return self.__y; }
    inline bigint order() const     { return self.__order; }

    inline bool operator==( const Point& other ) const;
    inline bool operator!=( const Point& other ) const;
    inline Point operator+( const Point& that ) const; // Add one point to another point.
    inline Point operator*( const bigint& other ) const;
    inline Point Double() const;
};
static const Point Point_INFINITY(CurveFp_NONE, bigint_NONE, bigint_NONE, bigint_NONE);

Point::Point() :
    __curve(CurveFp_NONE),
    __x(bigint_NONE),
    __y(bigint_NONE),
    __order(bigint_NONE)
{}

Point::Point( const Point& other ) :
    __curve(other.__curve),
    __x(other.__x),
    __y(other.__y),
    __order(other.__order)
{}

Point::Point( const CurveFp& curve, const bigint& x, const bigint& y, const bigint& order )
{
    // curve, x, y, order; order (optional) is the order of this point.
    self.__curve = curve;
    self.__x = x;
    self.__y = y;
    self.__order = order;
    // self.curve is allowed to be None only for INFINITY:
    if( self.__curve!=CurveFp_NONE )
        ASSERT( self.__curve.contains_point(x, y) );
    if( order!=bigint_NONE )
        ASSERT( (self * order) == Point_INFINITY );
}


bool Point::operator==( const Point& other ) const
{
    return ((self.__curve==other.__curve) && (self.__x == other.__x) && (self.__y == other.__y));
}

bool Point::operator!=( const Point& other ) const
{
    return !((self.__curve==other.__curve) && (self.__x == other.__x) && (self.__y == other.__y));
}

// Add one point to another point.
Point Point::operator+( const Point& other ) const
{
    // X9.62 B.3:

    if( other == Point_INFINITY )
        return self;
    if( self == Point_INFINITY )
        return other;
    ASSERT( self.__curve == other.__curve );
    if( self.__x == other.__x )
    {
        if( ((self.__y + other.__y) % self.__curve.p()) == 0 )
            return Point_INFINITY;
        else
            return self.Double();
    }
    bigint p = self.__curve.p();

    bigint l_PART1 = (other.__y - self.__y);
    bigint l_PART2 = inverse_mod(other.__x - self.__x, p);
    bigint l = (l_PART1 * l_PART2) % p;
    //bigint l = ((other.__y - self.__y) *
    //        inverse_mod(other.__x - self.__x, p)) % p;

    bigint x3 = (l * l - self.__x - other.__x) % p;
    bigint y3 = (l * (self.__x - x3) - self.__y) % p;

    return Point(self.__curve, x3, y3);
}

// Helper function
bigint leftmost_bit( const bigint& x )
{
    ASSERT( x > 0 );
    bigint result( 1 );
    //printf("leftmost_bit() x=%s \n",x.str().c_str());
    while( result <= x )
    {
        result = 2 * result;
        //printf("leftmost_bit() result=%s \n",result.str().c_str());
    }
    return (result / 2);
}

// Multiply a point by an integer.
Point Point::operator*(const bigint& other) const
{
    //printf("Point::operator*() self.xy=(%s,%s) \n",
    //    self.x().str().c_str(), self.y().str().c_str() );
    //printf("Point::operator*() other=%s self.__order=%s \n",
    //    other.str().c_str(), self.__order.str().c_str() );
    bigint e = other;
    if( self.__order!=bigint_NONE )
        e = e % self.__order;
    if( e == 0 )
        return Point_INFINITY;
    if( self == Point_INFINITY )
        return Point_INFINITY;
    ASSERT( e > 0 );

    // From X9.62 D.3.2:

    //printf("Point::operator*() e=%s \n", e.str().c_str() );
    bigint e3 = 3 * e;
    //printf("Point::operator*() e3=%s \n", e3.str().c_str() );
    Point negative_self = Point(self.__curve, self.__x, -self.__y, self.__order);
    //printf("Point::operator*() . . . negative_self = (%s,%s) \n",
    //    negative_self.x().str().c_str(), negative_self.y().str().c_str() );
    bigint i = (leftmost_bit(e3) / 2);
    //printf("Point::operator*() leftmost_bit(e3)=%s \n", i.str().c_str() );
    Point result = self;
    //print_("Multiplying %s by %d (e3 = %d):" % (self, other, e3))
    while( i > 1 )
    {
        result = result.Double();
        if( ((e3 & i) != 0) && ((e & i) == 0) )
            result = result + self;
        if( ((e3 & i) == 0) && ((e & i) != 0) )
            result = result + negative_self;
        //printf("Point::operator*() . . . i = %s, result = (%s,%s) \n",
        //    i.str().c_str(), result.x().str().c_str(), result.y().str().c_str() );
        i = i / 2;
    }

    return result;
}

Point operator*(const bigint& other, const Point& self_)
{
    return (self_ * other);
}

// Return a new point that is twice the old.
Point Point::Double() const
{

    if( self == Point_INFINITY )
      return Point_INFINITY;

    // X9.62 B.3:

    bigint p = self.__curve.p();
    bigint a = self.__curve.a();

    bigint l = ((3 * self.__x * self.__x + a) *
         inverse_mod(2 * self.__y, p)) % p;

    bigint x3 = (l * l - 2 * self.__x) % p;
    bigint y3 = (l * (self.__x - x3) - self.__y) % p;
    //printf("Point::Double() . . . p=%s a=%s l=%s \n",
    //    p.str().c_str(), a.str().c_str(), l.str().c_str() );
    //printf("Point::Double() . . . x3=%s y3=%s \n",
    //    x3.str().c_str(), y3.str().c_str() );

    return Point(self.__curve, x3, y3);
}

struct Signature {
    bigint r;
    bigint s;
    Signature( const bigint& r, const bigint& s )
    {
        self.r = r;
        self.s = s;
    }
};

// Public key for ECDSA.
struct Public_key {
    CurveFp curve;
    Point generator;
    Point point;

    Public_key( const Point& generator, const Point& point )
    {
        self.curve = generator.curve();
        self.generator = generator;
        self.point = point;
        bigint n = generator.order();
        if( n==bigint_NONE )
          printf("Generator point must have order. \n");
        if( (n * point) != Point_INFINITY )
          printf("Generator point order is bad. \n");
        if( (point.x() < 0) || (n <= point.x()) || (point.y() < 0) || (n <= point.y()) )
          printf("Generator point has x or y out of range. \n");
    }

    bool verifies(const bigint& hash, const Signature& signature); // True if valid signature of hash
};

// Verify that signature is a valid signature of hash.
// Return True if the signature is valid.
bool Public_key::verifies(const bigint& hash, const Signature& signature)
{
    // From X9.62 J.3.1.

    Point G = self.generator;
    bigint n = G.order();
    bigint r = signature.r;
    bigint s = signature.s;
    if( (r < 1) || (r > (n - 1)) )
        return false;
    if( (s < 1) || (s > (n - 1)) )
        return false;
    bigint c = inverse_mod(s, n);
    //printf( "verifies(), c=%s %s \n", c.str().c_str(), (c.type==0?"NORMAL":"NONE") );
    bigint u1 = (hash * c) % n;
    bigint u2 = (r * c) % n;
    //printf( "verifies(), (hash * c)=%s, u1=%s %s \n",
    //    (hash*c).str().c_str(), u1.str().c_str(), (u1.type==0?"NORMAL":"NONE") );
    //printf( "verifies(), (r * c)=%s, u2=%s %s \n",
    //    (r*c).str().c_str(), u2.str().c_str(), (u2.type==0?"NORMAL":"NONE") );
    //printf( "verifies(), G=(%s,%s) \n", G.x().str().c_str(), G.y().str().c_str() );
    //printf( "verifies(), self.point=(%s,%s) \n",
    //    self.point.x().str().c_str(), self.point.y().str().c_str() );
    Point xy = (u1 * G);
    //printf( "verifies(), xy=(%s,%s) \n", xy.x().str().c_str(), xy.y().str().c_str() );
    xy = xy + (u2 * self.point);
    //printf( "verifies(), xy=(%s,%s) \n", xy.x().str().c_str(), xy.y().str().c_str() );
    bigint v = xy.x() % n;
    return (v == r);
}

// Private key for ECDSA.
//struct Private_key {
//    Public_key public_key;
//    bigint secret_multiplier;
//    Private_key( const Public_key& public_key, bigint secret_multiplier )
//    {
//        self.public_key = public_key;
//        self.secret_multipler = secret_multiplier;
//    }
//
//    Signature sign( const bigint& hash, const bigint& random_k ) // Return a signature for a hash
//};

// Return a signature for the provided hash, using the provided
// random nonce.  It is absolutely vital that random_k be an unpredictable
// number in the range [1, self.public_key.point.order()-1].  If
// an attacker can guess random_k, he can compute our private key from a
// single signature.  Also, if an attacker knows a few high-order
// bits (or a few low-order bits) of random_k, he can compute our private
// key from many signatures.  The generation of nonces with adequate
// cryptographic strength is very difficult and far beyond the scope
// of this comment.
// May raise RuntimeError, in which case retrying with a new
// random value k is in order.
//Signature Private_key::sign( const bigint& hash, const bigint& random_k )
//{
//    Point G = self.public_key.generator;
//    bigint n = G.order();
//    bigint k = random_k % n;
//    Point p1 = k * G;
//    bigint r = p1.x() % n;
//    if( r == 0 )
//      printf("amazingly unlucky random number r \n");
//    bigint s = (inverse_mod(k, n) *
//         (hash + (self.secret_multiplier * r) % n)) % n;
//    if( s == 0 ):
//      printf("amazingly unlucky random number s \n");
//    return Signature(r, s)
//}

//---------- ----------
// Mini curve - 40-bit encryption
//---------- ----------

// ORIGINAL PARAMS
// downloaded from https://bountify.co/implementation-of-ecdsa-or-eddsa-that-generates-80-bit-private-keys
// _p = 989292113647 # Original
// _a = -3 # original
// _b = 667230586810 # Original
// _r = 989290653721 # Original
// _Gx = 762284522849 # Original
// _Gy = 820883475793 # Original

// CUSTOM PARAMS
// GENERATED BY MODFIED ECGEN
// downloaded from https://github.com/J08nY/ecgen
// then modified to search for order value _r which is prime,
// using random 40-bit _b and random prime 40-bit _p values
bigint _p = "989292117823"; // BWAHAHA MY NUMBER - from modified ecgen, random 40-bit prime
bigint _a = "-3"; // Original
bigint _b = "911333413149"; // BWAHAHA MY NUMBER - from modified ecgen, random 40-bit integer
bigint _r = "989291303419"; // from modified ecgen 
bigint _Gx = "146885098810"; // from modified ecgen
bigint _Gy = "687327530143"; // from modified ecgen


CurveFp curve_mini = CurveFp(_p, _a, _b);
Point generator_mini = Point(curve_mini, _Gx, _Gy, _r);


//---------- ----------
// Salt and public key for this application
//---------- ----------

#ifdef PSDTO3D_FULL_VERSION
const int IsFullVersion = 1;
#else
const int IsFullVersion = 0;
#endif

#if defined PSDTO3D_FBX_VERSION
const int IsStandaloneVersion = 1;
#else
const int IsStandaloneVersion = 0;
#endif

const int64_t salt_len = 64;

const int64_t salt_ver_standalone[salt_len] =
{
    922382856,  38489343,   495722429,  446973736,  244161361,  1159049220, 806466900,  1607589334,
    1742627586, 423079096,  1215251532, 1238533909, 1896114477, 869478465,  918658266,  1522517298,
    1393417026, 1632794218, 89788771,   329736960,  678904445,  1172599155, 575285451,  1996980024,
    365844601,  984671920,  1465981678, 1284228559, 1659409551, 993429976,  278259903,  750126780,
    607450463,  1976982879, 1373067555, 1258071601, 468022779,  2042320011, 66349091,   1669024814,
    683875742,  101686246,  1377374241, 1963524129, 1869903615, 471636841,  536704152,  325973376,
    1491409034, 565938618,  246523098,  1844559119, 1037357760, 501051505,  691927227,  1894691645,
    145728359,  177997881,  1284555924, 976073106,  1395419312, 169327264,  1418767404, 2125063435
};
const int64_t salt_ver_full[salt_len] =
{
    680870879,  1949418040, 659128673,  804644935,  1628735301, 1887046749, 48502696,   1021427000,
    66717462,   226983266,  274008246,  1290557923, 1106553353, 321994718,  2140052265, 1192847408,
    204460187,  881886298,  1946193129, 2009080486, 806231867,  1612017863, 1023258161, 273188057,
    687948726,  7626498,    1020012050, 131629508,  400947585,  2133079167, 1848022864, 1950949787,
    18946239,   780153552,  1116752631, 1654810922, 1431583066, 1569536426, 1086101237, 574778060,
    1947251708, 1726768480, 617878230,  1693820478, 746980759,  1566068608, 739635038,  75289780,
    1566097374, 1637610293, 492772817,  355248210,  152200218,  1224186337, 1264564052, 256593157,
    247172359,  1405001852, 1190138969, 335041368,  1209700842, 2133031942, 1116983806, 398044237
};

const int64_t salt_ver_lite[salt_len] =
{
    813112671,  1883524514, 694257224,  1280117951, 1610094302, 770385160,  421501030,  2095834128,
    1289557814, 318057935,  1953782206, 444408167,  750249778,  230850776,  1165068004, 1483214611,
    2029123854, 927109088,  597146711,  367170135,  1250454830, 668461008,  823920420,  2003608135,
    285023875,  32591367,   118159258,  1522601046, 1988386656, 1649608491, 1293228984, 97726923,
    760946384,  193038754,  1228295524, 2869189,    1365670316, 40939251,   1135817258, 602951234,
    1593504794, 131948983,  1402759508, 734660184,  263420357,  975427435,  567803220,  2022244682,
    1567993272, 1277667709, 1582177999, 1224078323, 1400022421, 1275282537, 942330488,  6196806,
    1154599941, 865882220,  432913764,  1988627999, 1807574178, 1377586192, 2007807467, 1817990012
};


const bigint public_key_ver_standalone[2] = { "875497492320", "533017399275" };
const bigint public_key_ver_full[2] = { "271050622680", "788410928891" };
const bigint public_key_ver_lite[2] = { "296467825117", "951542512370" };

//#define ECDSA_TEST 1
#ifdef ECDSA_TEST
    //---------- ----------
    // Test
    //---------- ----------
    const int64_t salt[salt_len] =
    {
        1148366987, 486981732,  559319269,  233986369,  1249669079, 451458150,  1387434145, 759772964,
        916928826,  1777550046, 852907713,  628896737,  749417669,  431609759,  1349610418, 609757873,
        820684389,  1545731636, 69377515,   1600509069, 1286799653, 826830676,  603250913,  716471583,
        1103527485, 1114276294, 2013787736, 1119096095, 1773478280, 225091660,  728177064,  237547622,
        2053837500, 339878229,  241529008,  1409908579, 1086664349, 493065756,  1422925608, 801806910,
        1580070569, 887419632,  1170602527, 1423359080, 2104696046, 373336314,  396612453,  1467087636,
        1566152781, 831104344,  1859918116, 1185287231, 1151219577, 1809273101, 1736902566, 791420981,
        1490124492, 377004830,  722995779,  91147107,   1803639607, 1429525131, 232631073,  606137760,
    };
    const bigint public_key[2] = { "870996650127", "723436460596" };
#else
const int64_t* salt =
	(IsStandaloneVersion? salt_ver_standalone :
		(IsFullVersion? salt_ver_full:salt_ver_lite));
const bigint* public_key =
	(IsStandaloneVersion? public_key_ver_standalone :
		(IsFullVersion? public_key_ver_full:public_key_ver_lite));
#endif


//---------- ----------
// Helpers
//---------- ----------

// Helper
int64_t max(int64_t a, int64_t b)
{
    return (a>=b? a:b);
}

int64_t rand_64()
{
    union
    {
        unsigned char b[8];
        int64_t s;
    };
    for( size_t i=0; i<8; i++ )
    {
        b[i] = (unsigned char)(rand() & 0xFF);
    }
    return s;
}
// random number, maximum up to a given number of bits, may be positive or negative
// currently only supports multiples of 10 bits
bigint rand_bigint( const int bits )
{
    bigint retval(0);
    // each iteration shift in another 10-bit value
    for( size_t i=0; i<(bits/10); i++ )
    {
        retval = retval * 1024; // 10-bits = 1024
        retval = retval + (rand() % 1024);
    }
    // return a negative number half the time
    //if( rand()<(RAND_MAX/2) )
    //    retval = -retval;
    return retval;
}


// Helper
//bool is_upper( char c )
//{ return ((c>='A') && (c<='Z')); }
//bool is_whitespace( char c )
//{ return ((c==' ') || (c=='\t') || (c=='\n')); }

// Helper
//char to_lower( char c ) // DOESN'T WORK WITH UTF-8 ENCODING - AVOID
//{
//    static const char offset = ('A'-'a'); // offset from uppercase to lowercase range in ASCII
//    if( is_upper(c) )
//        return (c - offset); // uppercase, return with offset applied
//    return (c); // not uppercase, return as is
//}

// Helper
//struct StringConform // DOESN'T WORK WITH UTF-8 ENCODING - AVOID
//{
//    char* str;
//    size_t length;
//    StringConform( const char* str_in )
//    {
//        length = strlen(str_in) + 1; // add one for terminating character
//        str = new char[length];
//        size_t front=0; // first char of input string
//        for( ; str_in[front]!=0; front++ ) // strip leading whitespace
//            if( !is_whitespace(str_in[front]) ) break;
//        strcpy_s( str, length-front, str_in+front );
//        size_t back=(length-front)-2; // last char of output string
//        for( ; back>=0; back-- ) // strip trailing whitespace
//            if( !is_whitespace(str[back]) ) break;
//            else str[back]='\0';
//    }
//    ~StringConform()
//    { delete[] str; }
//};

// Helper
size_t split_hex( int64_t* res, size_t res_len, const char* text, const char delim )
{
    //int64_t test;
    //sscanf_s( "0x01FFFFFFFF", "%llX", &test );

    size_t res_pos = 0;
    size_t buf_pos = 3;
    const size_t buf_size = 64;
    char buf[buf_size] = "0x0"; // first characters are hex tag
    for( const char* s = text; (*s)!='\0'; s++ )
    {
        if( (s[0]!=delim) && (s[0]!='\0') )
            buf[buf_pos++] = (*s);
        if( (s[1]==delim) || (s[1]=='\0') )
        {
            buf[buf_pos] = '\0';
            sscanf_s( buf, "%llX", &(res[res_pos]) );
            res_pos++;
            buf_pos = 3; // rewind to hex tag
        }
    }
    return res_pos;
}

size_t split_bigint( bigint* res, size_t res_len, const char* text, int base, const char delim )
{
    //int64_t test;
    //sscanf_s( "0x01FFFFFFFF", "%llX", &test );

    size_t res_pos = 0;
    size_t buf_pos = 3;
    const size_t buf_size = 64;
    char buf[buf_size] = "0x0"; // first characters are hex tag
    for( const char* s = text; (*s)!='\0'; s++ )
    {
        if( (s[0]!=delim) && (s[0]!='\0') )
            buf[buf_pos++] = (*s);
        if( (s[1]==delim) || (s[1]=='\0') )
        {
            buf[buf_pos] = '\0';
            if( base==16 ) // read with hex tag if base 16
                res[res_pos] = buf;
            else if( base==10 ) // read without hex tag if base 10
                res[res_pos] = (buf+2);
            //sscanf_s( buf, "%llX", &(res[res_pos]) );
            res_pos++;
            buf_pos = 3; // rewind to hex tag
        }
    }
    return res_pos;
}


//---------- ----------
// Hash routines
//---------- ----------

bigint compute_hash( const unsigned char* text, int64_t text_len )
{
    // Compute the mash, text + salt compressed into fixed-sized list
    // array of integers, fixed size equal to length of salt list
    // text byte added to each salt value, loop around if too long
    //
    int64_t mash[salt_len];
    for( int64_t i=0; i<salt_len; i++ ) mash[i] = 0;

    //text = bytearray(text);
    //int64_t text_len = strlen( (char*)text );
    int64_t scan_len = max( text_len, salt_len );
    for( int64_t i=0; i<scan_len; i++ )
    {
        int64_t c = text[i]; // Assume already converted to lowercase
        //int64_t c = to_lower(text[i]); // Convert text to lowercase
        int64_t j = i % 64;
        if( i<salt_len )
            mash[j] = long(salt[j]);
        if( i<text_len )
            mash[j] = mash[j] + (int64_t)(c);
        mash[j] = mash[j] % (int64_t)(2147483647); // clip to 32-bit integer
    }
    //
    // Compute the mash bytes
    // bytearray of the mash integers, four bytes per salt list value
    // ugh, manually convert hash to bytes
    //
    int64_t mash_len = (salt_len*4);
    unsigned char mash_bytes[(salt_len*4)];
    for( int i=0; i<salt_len; i++ )
    {
        int64_t j = i*4;
        mash_bytes[j+0] = (unsigned char)((mash[i] & 0x000000FF));
        mash_bytes[j+1] = (unsigned char)((mash[i] & 0x0000FF00) >> 8);
        mash_bytes[j+2] = (unsigned char)((mash[i] & 0x00FF0000) >> 16);
        mash_bytes[j+3] = (unsigned char)((mash[i] & 0xFF000000) >> 24);
    }
    //
    // Compute the hash
    // Finally!
    //
    unsigned char hash_bytes[32];
    calc_sha_256( hash_bytes, mash_bytes, mash_len);

    //
    // Compute the hash integer
    // integer with the first forty bits of the hash value
    // ugh, manually convert hash bytes to 40-bit integer
    //
    bigint hash = 0;
    hash += ((int64_t)hash_bytes[4]);
    hash *= 256;
    hash += ((int64_t)hash_bytes[3]);
    hash *= 256;
    hash += ((int64_t)hash_bytes[2]);
    hash *= 256;
    hash += ((int64_t)hash_bytes[1]);
    hash *= 256;
    hash += ((int64_t)hash_bytes[0]);
    //hash = hash % 0xFFFFFFFFFF; // 40-bit hash
    //
    return hash;
}

//---------- ----------
// Main routine
//---------- ----------

//int check_string( unsigned char* a )
//{
//    unsigned char b[] =
//    {
//        207, 131, 206, 186,     207, 141, 206, 187,
//        206, 191, 207, 130,     32, 208, 186, 208,
//        190, 209, 130, 32,      109, 195, 186, 115,
//        32, 235, 167, 144,      32, 231, 140, 180,
//        229, 173, 144, 32,      227, 131, 169, 227,
//        131, 147, 227, 131,     131, 227, 131, 136,
//        38, 115, 121, 109,      98, 111, 108, 32,
//        34, 96, 47, 92,         45, 42, 37, 94,
//        36, 38, 64, 33,         126, 43, 61, 40,
//        91, 123, 60, 42,        63, 62, 125, 93,
//        41, 58, 59, 44,         46, 9, 9, 34,
//        96, 47, 92, 45,         42, 37, 94, 36,
//        38, 64, 33, 126,        43, 61, 40, 91,
//        123, 60, 42, 63,        62, 125, 93, 41,
//        58, 59, 44, 46,         32, 115, 121, 109,
//        98, 111, 108, 38,       109, 105, 99, 104,
//        97, 101, 108, 115,      111, 110, 98, 114,
//        105, 116, 116, 64,      99, 115, 46, 99,
//        111, 109
//    };
//    for( int i=0; i<strlen((char*)a); i++ )
//        if( a[i]!=b[i] )
//            return 0;
//    return 1;
//}

int ECDSA_Verify( const char* first_name, const char* last_name, const char* email, const char* license_key )
{
    bigint signature_items[2];
    size_t sig_len = split_bigint(signature_items, 2, license_key, 16, '-' ); // base16 hex
    if( sig_len != 2 )
        return 0; //printf( "ERROR: expected serial number in format 0123456789-0123456789 \n" );
    else
    {
        Signature signature = Signature( signature_items[0], signature_items[1] );
        //printf( "serial number: %s-%s \n", signature.r.str(16).c_str(), signature.s.str(16).c_str() );
        //
        Point g = generator_mini;
        bigint n = g.order();
        bigint pointx = public_key[0];
        bigint pointy = public_key[1];
        Point keypoint = Point( curve_mini, pointx, pointy, n );
        Public_key pubkey = Public_key( g, keypoint );
        //printf( "public key: %s:%s \n", pubkey.point.x().str().c_str(), pubkey.point.y().str().c_str() );
        //

        // Assume already trimmed whitespace
        //StringConform detail_1(first_name);
        //StringConform detail_2(last_name);
        //StringConform detail_3(email);

        // Build message text as "first&last&email"
        // For example, Art Bee artbee@email.com becomes Art&Bee&artbee.email.com
        const char* detail_1 = first_name;
        const char* detail_2 = last_name;
        const char* detail_3 = email;
        // TODO: does strlen() support UTF-8 encoding?
        size_t total_len = (strlen(detail_1) + strlen(detail_2) + strlen(detail_3) + 2 + 1); // add two for & chars, one for terminating zero
        char* msg = new char[total_len+16]; // add another sixteen for good measure, why not
        sprintf_s(msg, total_len, "%s&%s&%s", detail_1, detail_2, detail_3);

        //check_string( (unsigned char*)msg );

        // Compute the hash of the message
        bigint hash = compute_hash( (unsigned char*)msg, (int64_t)total_len );
        delete[] msg;

        //printf( "hash: %s \n", hash.str().c_str() );
        //
        if( pubkey.verifies( hash, signature ) )
            return 1; //printf("SUCCESSFULLY OPENED - CORRECT SIGNATURE \n");

        return 0; //    printf("FAIL TO OPEN - WRONG SIGNATURE \n");
    }
}

// # Python main function
//int main( int argc, char** argv )
//{
//    //printf(" _p = %s \n", _p.str().c_str() );
//    //printf(" _a = %s \n", _a.str().c_str() );
//    //printf(" _b = %s \n", _b.str().c_str() );
//    //printf(" _r = %s \n", _r.str().c_str() );
//    //printf(" _Gx = %s \n", _Gx.str().c_str() );
//    //printf(" _Gy = %s \n", _Gy.str().c_str() );
//    //bigint test1 = "0x010";
//    //printf(" test1 = 0x%s / %s \n", test1.str(16).c_str(), test1.str(10).c_str() );
//
//    bool print_usage = false;
//    if( argc<2 )
//        print_usage = true;
//    else if( (strcmp(argv[1],"gen")==0) && (argc==2) )
//    {
//        printf("generating keys... \n");
//
//        printf("NOT IMPLEMENTED \n");
//        //randrange = SystemRandom().randrange
//        //g = generator_mini
//        //n = g.order()
//        //secret = randrange( 1, n )
//        //pubkey = Public_key( g, g * secret )
//        //privkey = Private_key( pubkey, secret )
//        //#print( "public key: "+format(pubkey.point.x(),'02x')+":"+format(pubkey.point.y(),'02x') );
//        //#print( "secret key: "+format(privkey.secret_multiplier,'02x') );
//        //with open('secret_key.txt', 'w') as filehandle:  
//        //    filehandle.write( format(privkey.secret_multiplier) )
//        //    filehandle.write('\n')
//        //    filehandle.write( format(pubkey.point.x()) )
//        //    filehandle.write('\n')
//        //    filehandle.write( format(pubkey.point.y()) )
//        //    filehandle.write('\n')
//        //with open('public_key.txt', 'w') as filehandle:  
//        //    filehandle.write( format(pubkey.point.x()) )
//        //    filehandle.write('\n')
//        //    filehandle.write( format(pubkey.point.y()) )
//        //    filehandle.write('\n')
//        //with open('salt_key.txt', 'w') as filehandle:
//        //    for i in range(0,salt_len): # iterate 0..(salt code length-1)
//        //        filehandle.write( format(randrange( 1, 2147483647 )) )
//        //        filehandle.write('\n')
//        //printf("done. \n");
//    }
//    else if( (strcmp(argv[1],"sign")==0) && (argc==3) )
//    {
//        printf("signing message... \n");
//
//        printf("NOT IMPLEMENTED \n");
//        //with open('secret_key.txt', 'r') as filehandle:
//        //    data = filehandle.readlines()
//        //#
//        //g = generator_mini
//        //n = g.order()
//        //secret = long(data[0])
//        //pointx = long(data[1])
//        //pointy = long(data[2])
//        //keypoint = Point( curve_mini, pointx, pointy, n )
//        //pubkey = Public_key( g, keypoint )
//        //privkey = Private_key( pubkey, secret )
//        //#print( "public key: "+format(pubkey.point.x(),'02x')+":"+format(pubkey.point.y(),'02x') );
//        //#print( "secret key: "+format(privkey.secret_multiplier,'02x') );
//        //#
//        //hash = compute_hash( sys.argv[2] )
//        //#
//        //signature = privkey.sign( hash, randrange( 1, n ) )
//        //#print( "hash: " + str(hash) )
//        //#print( "serial number: " + format(signature.r,'02x') + '-' + format(signature.s,'02x') )
//        //print( format(signature.r,'02x') + '-' + format(signature.s,'02x') )
//    }
//    else if( (strcmp(argv[1],"open")==0) && (argc==4) )
//    {
//        bigint signature_items[2];
//        size_t sig_len = split_bigint(signature_items, 2, argv[2], 16, '-' ); // base16 hex
//        if( sig_len != 2 )
//            printf( "ERROR: expected serial number in format 0123456789-0123456789 \n" );
//        else
//        {
//            Signature signature = Signature( signature_items[0], signature_items[1] );
//            printf( "serial number: %s-%s \n", signature.r.str(16).c_str(), signature.s.str(16).c_str() );
//            //
//            Point g = generator_mini;
//            bigint n = g.order();
//            bigint pointx = public_key[0];
//            bigint pointy = public_key[1];
//            Point keypoint = Point( curve_mini, pointx, pointy, n );
//            Public_key pubkey = Public_key( g, keypoint );
//            printf( "public key: %s:%s \n", pubkey.point.x().str().c_str(), pubkey.point.y().str().c_str() );
//            //
//            bigint hash = compute_hash( argv[3] );
//            printf( "hash: %s \n", hash.str().c_str() );
//            //
//            if( pubkey.verifies( hash, signature ) )
//                printf("SUCCESSFULLY OPENED - CORRECT SIGNATURE \n");
//            else
//                printf("FAIL TO OPEN - WRONG SIGNATURE \n");
//        }
//    }
//    else if( (strcmp(argv[1],"test")==0) && (argc==4) )
//    {
//        bigint signature_items[2];
//        size_t sig_len = split_bigint(signature_items, 2, argv[2], 16, '-' ); // base16 hex
//        if( sig_len != 2 )
//            printf( "ERROR: expected serial number in format 0123456789-0123456789 \n" );
//        else
//        {
//            Signature signature = Signature( signature_items[0], signature_items[1] );
//            Point g = generator_mini;
//            bigint n = g.order();
//            bigint pointx = public_key[0];
//            bigint pointy = public_key[1];
//            Point keypoint = Point( curve_mini, pointx, pointy, n );
//            Public_key pubkey = Public_key( g, keypoint );
//            bigint hash = compute_hash( argv[3] );
//
//            if( pubkey.verifies( hash, signature ) )
//                printf("BASIC TEST:  SUCCESSFULLY OPENED - CORRECT SIGNATURE \n");
//            else
//                printf("BASIC TEST: ERROR, FAIL TO OPEN - WRONG SIGNATURE \n");
//
//            printf( "Attempting brute force serial number generation, \n" );
//            printf( "will run forever, hit CTRL+Z to exit ... \n" );
//            
//            srand((unsigned int)time(0));
//            int64_t tamper_attempts = 0;
//            bool tampered = false;
//            while( !tampered ) // may loop forever until user breaks
//            {
//                // Create random signature
//                Signature signature_test = signature;
//                //signature_test.r += rand_bigint(40); // random 40-bit value
//                //signature_test.s += rand_bigint(40); // random 40-bit value
//                signature_test.r = rand_bigint(40); // random 40-bit value
//                signature_test.s = rand_bigint(40); // random 40-bit value
//                // Check the random signature
//                if( pubkey.verifies( hash, signature_test ) )
//                {
//                    printf( "**** Brute force succeeded after %lli tries, serial %s-%s",
//                        tamper_attempts, signature_test.r.str().c_str(), signature_test.s.str().c_str() );
//                    tampered = true;
//                }
//
//                tamper_attempts ++;
//                if( (tamper_attempts%1000)==0 )
//                {
//                    printf("most recent attempt = (%s,%s) \n",
//                        signature_test.r.str(16).c_str(), signature_test.s.str(16).c_str() );
//                    printf("attempts so far: %lli ... \n", tamper_attempts);
//                    //srand(time(0));
//                }
//            }
//        }
//
//        //Point g = generator_mini;
//        //bigint n = g.order()
//        //#
//        //secret = randrange( 1, n )
//        //pubkey = Public_key( g, g * secret )
//        //privkey = Private_key( pubkey, secret )
//        //# Signing a hash value:
//        //hash = randrange( 1, n )
//        //signature = privkey.sign( hash, randrange( 1, n ) )
//        //#
//        //# Verifying a signature for a hash value:
//        //if pubkey.verifies( hash, signature ):
//        //    print("Demo verification succeeded.")
//        //else:
//        //    print("*** Demo verification failed.")
//        //# Verification fails if the hash value is modified:
//        //#if pubkey.verifies( hash-1, signature ):
//        //tampered = False
//        //tampered_attempts = 500
//        //for i in range(0,tampered_attempts):
//        //    if pubkey.verifies( randrange(0,0xFFFFFFFFFF), signature ):
//        //        print("**** Demo verification failed to reject tampered hash after "+str(i)+"tries.")
//        //        tampered = True
//        //#
//        //if not tampered:
//        //    print("Demo verification correctly rejected tampered hash after "+str(tampered_attempts)+" tries.")
//    }
//    else
//        print_usage = true;
//
//	
//    if( print_usage )
//    {
//        printf( " \n" );
//        printf( " usage: \n" );
//        printf( " python ECDSA_Python.py [gen | sign FIRSTNAME&LASTNAME&EMAIL | open SIG TEXT] \n" );
//        printf( " \n" );
//        printf( " gen: writes secret_key.txt and public_key.txt text files, \n" );
//        printf( " don't lose these, and only generate new keys once per software release \n" );
//        printf( " \n" );
//        printf( " sign: pass utf8 FIRSTNAME, LASTNAME, and EMAIL, chars \"& converted to _ \n" );
//        printf( " and concatenate the three values together with & characters separating them \n" );
//        printf( " \n" );
//        printf( " sign: pass signature value from sign command, and same text from sign command \n" );
//        printf( " prints whether signature matches text \n" );
//        printf( " \n" );
//    }
//}

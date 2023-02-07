#---------- ----------
# Original from https://github.com/warner/python-ecdsa/blob/master/src/ecdsa/ellipticcurve.py
# Additional tools from https://github.com/J08nY/ecgen
# Modified by Michaelson Britt, March 2019
#---------- ----------

"""
Implementation of Elliptic-Curve Digital Signatures.
Classes and methods for elliptic-curve signatures:
private keys, public keys, signatures,
NIST prime-modulus curves with modulus lengths of
192, 224, 256, 384, and 521 bits.
Example:
  # (In real-life applications, you would probably want to
  # protect against defects in SystemRandom.)
  from random import SystemRandom
  randrange = SystemRandom().randrange
  # Generate a public/private key pair using the NIST Curve P-192:
  g = generator_192
  n = g.order()
  secret = randrange( 1, n )
  pubkey = Public_key( g, g * secret )
  privkey = Private_key( pubkey, secret )
  # Signing a hash value:
  hash = randrange( 1, n )
  signature = privkey.sign( hash, randrange( 1, n ) )
  # Verifying a signature for a hash value:
  if pubkey.verifies( hash, signature ):
    print_("Demo verification succeeded.")
  else:
    print_("*** Demo verification failed.")
  # Verification fails if the hash value is modified:
  if pubkey.verifies( hash-1, signature ):
    print_("**** Demo verification failed to reject tampered hash.")
  else:
    print_("Demo verification correctly rejected tampered hash.")
Version of 2009.05.16.
Revision history:
      2005.12.31 - Initial version.
      2008.11.25 - Substantial revisions introducing new classes.
      2009.05.16 - Warn against using random.randrange in real applications.
      2009.05.17 - Use random.SystemRandom by default.
Written in 2005 by Peter Pearson and placed in the public domain.
"""

from six import int2byte, b
from six import python_2_unicode_compatible
from six import integer_types
from six.moves import reduce

import math


class Error(Exception):
  """Base class 
 exceptions in this module."""
  pass


class SquareRootError(Error):
  pass


class NegativeExponentError(Error):
  pass


def modular_exp(base, exponent, modulus):
  "Raise base to exponent, reducing by modulus"
  if exponent < 0:
    raise NegativeExponentError("Negative exponents (%d) not allowed" \
                                % exponent)
  return pow(base, exponent, modulus)
#   result = 1L
#   x = exponent
#   b = base + 0L
#   while x > 0:
#     if x % 2 > 0: result = (result * b) % modulus
#     x = x // 2
#     b = (b * b) % modulus
#   return result


def polynomial_reduce_mod(poly, polymod, p):
  """Reduce poly by polymod, integer arithmetic modulo p.
  Polynomials are represented as lists of coefficients
  of increasing powers of x."""

  # This module has been tested only by extensive use
  # in calculating modular square roots.

  # Just to make this easy, require a monic polynomial:
  assert polymod[-1] == 1

  assert len(polymod) > 1

  while len(poly) >= len(polymod):
    if poly[-1] != 0:
      for i in range(2, len(polymod) + 1):
        poly[-i] = (poly[-i] - poly[-1] * polymod[-i]) % p
    poly = poly[0:-1]

  return poly


def polynomial_multiply_mod(m1, m2, polymod, p):
  """Polynomial multiplication modulo a polynomial over ints mod p.
  Polynomials are represented as lists of coefficients
  of increasing powers of x."""

  # This is just a seat-of-the-pants implementation.

  # This module has been tested only by extensive use
  # in calculating modular square roots.

  # Initialize the product to zero:

  prod = (len(m1) + len(m2) - 1) * [0]

  # Add together all the cross-terms:

  for i in range(len(m1)):
    for j in range(len(m2)):
      prod[i + j] = (prod[i + j] + m1[i] * m2[j]) % p

  return polynomial_reduce_mod(prod, polymod, p)


def polynomial_exp_mod(base, exponent, polymod, p):
  """Polynomial exponentiation modulo a polynomial over ints mod p.
  Polynomials are represented as lists of coefficients
  of increasing powers of x."""

  # Based on the Handbook of Applied Cryptography, algorithm 2.227.

  # This module has been tested only by extensive use
  # in calculating modular square roots.

  assert exponent < p

  if exponent == 0:
    return [1]

  G = base
  k = exponent
  if k % 2 == 1:
    s = G
  else:
    s = [1]

  while k > 1:
    k = k // 2
    G = polynomial_multiply_mod(G, G, polymod, p)
    if k % 2 == 1:
      s = polynomial_multiply_mod(G, s, polymod, p)

  return s


def jacobi(a, n):
  """Jacobi symbol"""

  # Based on the Handbook of Applied Cryptography (HAC), algorithm 2.149.

  # This function has been tested by comparison with a small
  # table printed in HAC, and by extensive use in calculating
  # modular square roots.

  assert n >= 3
  assert n % 2 == 1
  a = a % n
  if a == 0:
    return 0
  if a == 1:
    return 1
  a1, e = a, 0
  while a1 % 2 == 0:
    a1, e = a1 // 2, e + 1
  if e % 2 == 0 or n % 8 == 1 or n % 8 == 7:
    s = 1
  else:
    s = -1
  if a1 == 1:
    return s
  if n % 4 == 3 and a1 % 4 == 3:
    s = -s
  return s * jacobi(n % a1, a1)


def square_root_mod_prime(a, p):
  """Modular square root of a, mod p, p prime."""

  # Based on the Handbook of Applied Cryptography, algorithms 3.34 to 3.39.

  # This module has been tested for all values in [0,p-1] for
  # every prime p from 3 to 1229.

  assert 0 <= a < p
  assert 1 < p

  if a == 0:
    return 0
  if p == 2:
    return a

  jac = jacobi(a, p)
  if jac == -1:
    raise SquareRootError("%d has no square root modulo %d" \
                          % (a, p))

  if p % 4 == 3:
    return modular_exp(a, (p + 1) // 4, p)

  if p % 8 == 5:
    d = modular_exp(a, (p - 1) // 4, p)
    if d == 1:
      return modular_exp(a, (p + 3) // 8, p)
    if d == p - 1:
      return (2 * a * modular_exp(4 * a, (p - 5) // 8, p)) % p
    raise RuntimeError("Shouldn't get here.")

  for b in range(2, p):
    if jacobi(b * b - 4 * a, p) == -1:
      f = (a, -b, 1)
      ff = polynomial_exp_mod((0, 1), (p + 1) // 2, f, p)
      assert ff[1] == 0
      return ff[0]
  raise RuntimeError("No b found.")

# TODO: Delete this, testing only
inverse_mod_printed = False

def inverse_mod(a, m):
  """Inverse of a mod m."""

  global inverse_mod_printed
  if a < 0 or m <= a:
    #if not inverse_mod_printed:
    #  print("inverse_mod(), case 1")
    a = a % m
  #else:
  #  if not inverse_mod_printed:
  #    print("inverse_mod(), case 2")

  # From Ferguson and Schneier, roughly:

  c, d = a, m
  uc, vc, ud, vd = 1, 0, 0, 1
  while c != 0:
    q, c, d = divmod(d, c) + (c,)
    uc, vc, ud, vd = ud - q * uc, vd - q * vc, uc, vc
    #if not inverse_mod_printed:
    #  print("inverse_mod(), q="+str(q)+", c="+str(c)+", d="+str(d))

  # At this point, d is the GCD, and ud*a+vd*m = d.
  # If d == 1, this means that ud is a inverse.

  #if not inverse_mod_printed:
  #print("inverse_mod(), d="+str(d)+", ud="+str(ud)+", m="+str(m))
  inverse_mod_printed = True

  assert d == 1
  if ud > 0:
    return ud
  else:
    return ud + m


def gcd2(a, b):
  """Greatest common divisor using Euclid's algorithm."""
  while a:
    a, b = b % a, a
  return b


def gcd(*a):
  """Greatest common divisor.
  Usage: gcd([ 2, 4, 6 ])
  or:    gcd(2, 4, 6)
  """

  if len(a) > 1:
    return reduce(gcd2, a)
  if hasattr(a[0], "__iter__"):
    return reduce(gcd2, a[0])
  return a[0]


def lcm2(a, b):
  """Least common multiple of two integers."""

  return (a * b) // gcd(a, b)


def lcm(*a):
  """Least common multiple.
  Usage: lcm([ 3, 4, 5 ])
  or:    lcm(3, 4, 5)
  """

  if len(a) > 1:
    return reduce(lcm2, a)
  if hasattr(a[0], "__iter__"):
    return reduce(lcm2, a[0])
  return a[0]


def factorization(n):
  """Decompose n into a list of (prime,exponent) pairs."""

  assert isinstance(n, integer_types)

  if n < 2:
    return []

  result = []
  d = 2

  # Test the small primes:

  for d in smallprimes:
    if d > n:
      break
    q, r = divmod(n, d)
    if r == 0:
      count = 1
      while d <= n:
        n = q
        q, r = divmod(n, d)
        if r != 0:
          break
        count = count + 1
      result.append((d, count))

  # If n is still greater than the last of our small primes,
  # it may require further work:

  if n > smallprimes[-1]:
    if is_prime(n):   # If what's left is prime, it's easy:
      result.append((n, 1))
    else:               # Ugh. Search stupidly for a divisor:
      d = smallprimes[-1]
      while 1:
        d = d + 2               # Try the next divisor.
        q, r = divmod(n, d)
        if q < d:               # n < d*d means we're done, n = 1 or prime.
          break
        if r == 0:              # d divides n. How many times?
          count = 1
          n = q
          while d <= n:               # As long as d might still divide n,
            q, r = divmod(n, d)       # see if it does.
            if r != 0:
              break
            n = q                     # It does. Reduce n, increase count.
            count = count + 1
          result.append((d, count))
      if n > 1:
        result.append((n, 1))

  return result


def phi(n):
  """Return the Euler totient function of n."""

  assert isinstance(n, integer_types)

  if n < 3:
    return 1

  result = 1
  ff = factorization(n)
  for f in ff:
    e = f[1]
    if e > 1:
      result = result * f[0] ** (e - 1) * (f[0] - 1)
    else:
      result = result * (f[0] - 1)
  return result


def carmichael(n):
  """Return Carmichael function of n.
  Carmichael(n) is the smallest integer x such that
  m**x = 1 mod n for all m relatively prime to n.
  """

  return carmichael_of_factorized(factorization(n))


def carmichael_of_factorized(f_list):
  """Return the Carmichael function of a number that is
  represented as a list of (prime,exponent) pairs.
  """

  if len(f_list) < 1:
    return 1

  result = carmichael_of_ppower(f_list[0])
  for i in range(1, len(f_list)):
    result = lcm(result, carmichael_of_ppower(f_list[i]))

  return result


def carmichael_of_ppower(pp):
  """Carmichael function of the given power of the given prime.
  """

  p, a = pp
  if p == 2 and a > 2:
    return 2**(a - 2)
  else:
    return (p - 1) * p**(a - 1)


def order_mod(x, m):
  """Return the order of x in the multiplicative group mod m.
  """

  # Warning: this implementation is not very clever, and will
  # take a long time if m is very large.

  if m <= 1:
    return 0

  assert gcd(x, m) == 1

  z = x
  result = 1
  while z != 1:
    z = (z * x) % m
    result = result + 1
  return result


def largest_factor_relatively_prime(a, b):
  """Return the largest factor of a relatively prime to b.
  """

  while 1:
    d = gcd(a, b)
    if d <= 1:
      break
    b = d
    while 1:
      q, r = divmod(a, d)
      if r > 0:
        break
      a = q
  return a


def kinda_order_mod(x, m):
  """Return the order of x in the multiplicative group mod m',
  where m' is the largest factor of m relatively prime to x.
  """

  return order_mod(x, largest_factor_relatively_prime(m, x))


def is_prime(n):
  """Return True if x is prime, False otherwise.
  We use the Miller-Rabin test, as given in Menezes et al. p. 138.
  This test is not exact: there are composite values n for which
  it returns True.
  In testing the odd numbers from 10000001 to 19999999,
  about 66 composites got past the first test,
  5 got past the second test, and none got past the third.
  Since factors of 2, 3, 5, 7, and 11 were detected during
  preliminary screening, the number of numbers tested by
  Miller-Rabin was (19999999 - 10000001)*(2/3)*(4/5)*(6/7)
  = 4.57 million.
  """

  # (This is used to study the risk of false positives:)
  global miller_rabin_test_count

  miller_rabin_test_count = 0

  if n <= smallprimes[-1]:
    if n in smallprimes:
      return True
    else:
      return False

  if gcd(n, 2 * 3 * 5 * 7 * 11) != 1:
    return False

  # Choose a number of iterations sufficient to reduce the
  # probability of accepting a composite below 2**-80
  # (from Menezes et al. Table 4.4):

  t = 40
  n_bits = 1 + int(math.log(n, 2))
  for k, tt in ((100, 27),
                (150, 18),
                (200, 15),
                (250, 12),
                (300, 9),
                (350, 8),
                (400, 7),
                (450, 6),
                (550, 5),
                (650, 4),
                (850, 3),
                (1300, 2),
                ):
    if n_bits < k:
      break
    t = tt

  # Run the test t times:

  s = 0
  r = n - 1
  while (r % 2) == 0:
    s = s + 1
    r = r // 2
  for i in range(t):
    a = smallprimes[i]
    y = modular_exp(a, r, n)
    if y != 1 and y != n - 1:
      j = 1
      while j <= s - 1 and y != n - 1:
        y = modular_exp(y, 2, n)
        if y == 1:
          miller_rabin_test_count = i + 1
          return False
        j = j + 1
      if y != n - 1:
        miller_rabin_test_count = i + 1
        return False
  return True


def next_prime(starting_value):
  "Return the smallest prime larger than the starting value."

  if starting_value < 2:
    return 2
  result = (starting_value + 1) | 1
  while not is_prime(result):
    result = result + 2
  return result


smallprimes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41,
               43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
               101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
               151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
               199, 211, 223, 227, 229, 233, 239, 241, 251, 257,
               263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
               317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
               383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
               443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
               503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
               577, 587, 593, 599, 601, 607, 613, 617, 619, 631,
               641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
               701, 709, 719, 727, 733, 739, 743, 751, 757, 761,
               769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
               839, 853, 857, 859, 863, 877, 881, 883, 887, 907,
               911, 919, 929, 937, 941, 947, 953, 967, 971, 977,
               983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033,
               1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
               1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
               1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229]

miller_rabin_test_count = 0
@python_2_unicode_compatible
class CurveFp(object):
  """Elliptic Curve over the field of integers modulo a prime."""
  def __init__(self, p, a, b):
    """The curve of points satisfying y^2 = x^3 + a*x + b (mod p)."""
    self.__p = p
    self.__a = a
    self.__b = b

  def p(self):
    return self.__p

  def a(self):
    return self.__a

  def b(self):
    return self.__b

  def contains_point(self, x, y):
    """Is the point (x,y) on this curve?"""
    return (y * y - (x * x * x + self.__a * x + self.__b)) % self.__p == 0

  def __str__(self):
    return "CurveFp(p=%d, a=%d, b=%d)" % (self.__p, self.__a, self.__b)

class Point(object):
  """A point on an elliptic curve. Altering x and y is forbidding,
     but they can be read by the x() and y() methods."""
  def __init__(self, curve, x, y, order=None):
    """curve, x, y, order; order (optional) is the order of this point."""
    self.__curve = curve
    self.__x = x
    self.__y = y
    self.__order = order
    # self.curve is allowed to be None only for INFINITY:
    if self.__curve:
      assert self.__curve.contains_point(x, y)
    if order:
      assert self * order == INFINITY

  def __eq__(self, other):
    """Return True if the points are identical, False otherwise."""
    if self.__curve == other.__curve \
       and self.__x == other.__x \
       and self.__y == other.__y:
      return True
    else:
      return False

  def __add__(self, other):
    """Add one point to another point."""

    # X9.62 B.3:

    if other == INFINITY:
      return self
    if self == INFINITY:
      return other
    assert self.__curve == other.__curve
    if self.__x == other.__x:
      if (self.__y + other.__y) % self.__curve.p() == 0:
        return INFINITY
      else:
        return self.double()

    p = self.__curve.p()

    l = ((other.__y - self.__y) * \
         inverse_mod(other.__x - self.__x, p)) % p

    x3 = (l * l - self.__x - other.__x) % p
    y3 = (l * (self.__x - x3) - self.__y) % p

    return Point(self.__curve, x3, y3)

  def __mul__(self, other):
    """Multiply a point by an integer."""

    def leftmost_bit(x):
      assert x > 0
      result = 1
      #print("leftmost_bit() x="+str(x))
      while result <= x:
        result = 2 * result
        #print("leftmost_bit() result="+str(result))
      return result // 2

    #print("Point::operator*() self.xy=("+str(self.__x)+","+str(self.__y)+")")
    #print("Point::operator*() other="+str(other)+" self.__order="+str(self.__order))
    e = other
    if self.__order:
      e = e % self.__order
    if e == 0:
      return INFINITY
    if self == INFINITY:
      return INFINITY
    assert e > 0

    # From X9.62 D.3.2:

    #print("Point::operator*() e="+str(e))
    e3 = 3 * e
    #print("Point::operator*() e3="+str(e3))
    negative_self = Point(self.__curve, self.__x, -self.__y, self.__order)
    #print("Point::operator*() negative_self.xy=("+str(negative_self.__x)+","+str(negative_self.__y)+")")
    i = leftmost_bit(e3) // 2
    #print("Point::operator*() leftmost_bit(e3)="+str(i))
    result = self
    # print_("Multiplying %s by %d (e3 = %d):" % (self, other, e3))
    while i > 1:
      result = result.double()
      if (e3 & i) != 0 and (e & i) == 0:
        result = result + self
      if (e3 & i) == 0 and (e & i) != 0:
        result = result + negative_self
      #print("Point::operator*(). . . i = %d, result = %s" % ( i, result ))
      i = i // 2

    return result

  def __rmul__(self, other):
    """Multiply a point by an integer."""

    return self * other

  def __str__(self):
    if self == INFINITY:
      return "infinity"
    return "(%d,%d)" % (self.__x, self.__y)

  def double(self):
    """Return a new point that is twice the old."""

    if self == INFINITY:
      return INFINITY

    # X9.62 B.3:

    p = self.__curve.p()
    a = self.__curve.a()

    l = ((3 * self.__x * self.__x + a) * \
         inverse_mod(2 * self.__y, p)) % p

    x3 = (l * l - 2 * self.__x) % p
    y3 = (l * (self.__x - x3) - self.__y) % p
    #print("Point::Double() p="+str(p)+" a="+str(a)+" l="+str(l))
    #print("Point::Double() x3="+str(x3)+" y3="+str(y3))

    return Point(self.__curve, x3, y3)

  def x(self):
    return self.__x

  def y(self):
    return self.__y

  def curve(self):
    return self.__curve

  def order(self):
    return self.__order


# This one point is the Point At Infinity for all purposes:
INFINITY = Point(None, None, None)


class RSZeroError(RuntimeError):
    pass


class Signature(object):
  """ECDSA signature.
  """
  def __init__(self, r, s):
    self.r = r
    self.s = s

  def recover_public_keys(self, hash, generator):
    """Returns two public keys for which the signature is valid
    hash is signed hash
    generator is the used generator of the signature
    """
    curve = generator.curve()
    n = generator.order()
    r = self.r
    s = self.s
    e = hash
    x = r

    # Compute the curve point with x as x-coordinate
    alpha = (pow(x, 3, curve.p()) + (curve.a() * x) + curve.b()) % curve.p()
    beta = square_root_mod_prime(alpha, curve.p())
    y = beta if beta % 2 == 0 else curve.p() - beta

    # Compute the public key
    R1 = Point(curve, x, y, n)
    Q1 = inverse_mod(r, n) * (s * R1 + (-e % n) * generator)
    Pk1 = Public_key(generator, Q1)

    # And the second solution
    R2 = Point(curve, x, -y, n)
    Q2 = inverse_mod(r, n) * (s * R2 + (-e % n) * generator)
    Pk2 = Public_key(generator, Q2)

    return [Pk1, Pk2]

class Public_key(object):
  """Public key for ECDSA.
  """

  def __init__(self, generator, point):
    """generator is the Point that generates the group,
    point is the Point that defines the public key.
    """

    self.curve = generator.curve()
    self.generator = generator
    self.point = point
    n = generator.order()
    if not n:
      raise RuntimeError("Generator point must have order.")
    if not n * point == INFINITY:
      raise RuntimeError("Generator point order is bad.")
    if point.x() < 0 or n <= point.x() or point.y() < 0 or n <= point.y():
      raise RuntimeError("Generator point has x or y out of range.")

  def verifies(self, hash, signature):
    """Verify that signature is a valid signature of hash.
    Return True if the signature is valid.
    """

    # From X9.62 J.3.1.

    G = self.generator
    n = G.order()
    r = signature.r
    s = signature.s
    if r < 1 or r > n - 1:
      return False
    if s < 1 or s > n - 1:
      return False
    c = inverse_mod(s, n)
    #print( "verifies(), c="+str(c))
    u1 = (hash * c) % n
    u2 = (r * c) % n
    #print( "verifies(), (hash * c)="+str(hash * c)+", u1="+str(u1))
    #print( "verifies(), (r * c)="+str(r * c)+", u2="+str(u2))
    #print( "verifies(), G="+str(G))
    #print( "verifies(), self.point="+str(self.point))
    #xy = u1 * G + u2 * self.point
    xy = (u1 * G)
    #print( "verifies(), xy="+str(xy))
    xy = xy + (u2 * self.point)
    #print( "verifies(), xy="+str(xy))
    v = xy.x() % n
    return v == r


class Private_key(object):
  """Private key for ECDSA.
  """

  def __init__(self, public_key, secret_multiplier):
    """public_key is of class Public_key;
    secret_multiplier is a large integer.
    """

    self.public_key = public_key
    self.secret_multiplier = secret_multiplier

  def sign(self, hash, random_k):
    """Return a signature for the provided hash, using the provided
    random nonce.  It is absolutely vital that random_k be an unpredictable
    number in the range [1, self.public_key.point.order()-1].  If
    an attacker can guess random_k, he can compute our private key from a
    single signature.  Also, if an attacker knows a few high-order
    bits (or a few low-order bits) of random_k, he can compute our private
    key from many signatures.  The generation of nonces with adequate
    cryptographic strength is very difficult and far beyond the scope
    of this comment.
    May raise RuntimeError, in which case retrying with a new
    random value k is in order.
    """

    G = self.public_key.generator
    n = G.order()
    k = random_k % n
    p1 = k * G
    r = p1.x() % n
    if r == 0:
      raise RSZeroError("amazingly unlucky random number r")
    s = (inverse_mod(k, n) *
         (hash + (self.secret_multiplier * r) % n)) % n
    if s == 0:
      raise RSZeroError("amazingly unlucky random number s")
    return Signature(r, s)


def int_to_string(x):
  """Convert integer x into a string of bytes, as per X9.62."""
  assert x >= 0
  if x == 0:
    return b('\0')
  result = []
  while x:
    ordinal = x & 0xFF
    result.append(int2byte(ordinal))
    x >>= 8

  result.reverse()
  return b('').join(result)


def string_to_int(s):
  """Convert a string of bytes into an integer, as per X9.62."""
  result = 0
  for c in s:
    if not isinstance(c, int):
      c = ord(c)
    result = 256 * result + c
  return result


def digest_integer(m):
  """Convert an integer into a string of bytes, compute
     its SHA-1 hash, and convert the result to an integer."""
  #
  # I don't expect this function to be used much. I wrote
  # it in order to be able to duplicate the examples
  # in ECDSAVS.
  #
  from hashlib import sha1
  return string_to_int(sha1(int_to_string(m)).digest())


def point_is_valid(generator, x, y):
  """Is (x,y) a valid public key based on the specified generator?"""

  # These are the tests specified in X9.62.

  n = generator.order()
  curve = generator.curve()
  if x < 0 or n <= x or y < 0 or n <= y:
    return False
  if not curve.contains_point(x, y):
    return False
  if not n * Point(curve, x, y) == INFINITY:
    return False
  return True


# curve of points satisfying y^2 = x^3 + a*x + b (mod p)
  

# ---------- ----------
# Mini curve - 40-bit encryption
# ---------- ----------

# ORIGINAL PARAMS
# downloaded from https://bountify.co/implementation-of-ecdsa-or-eddsa-that-generates-80-bit-private-keys
#_p = 989292113647 # Original
#_a = -3 # original
#_b = 667230586810 # Original
#_r = 989290653721 # Original
#_Gx = 762284522849 # Original
#_Gy = 820883475793 # Original

# CUSTOM PARAMS
# GENERATED BY MODFIED ECGEN
# downloaded from https://github.com/J08nY/ecgen
# then modified to search for order value _r which is prime,
# using random 40-bit _b and random prime 40-bit _p values
_p = 989292117823 # BWAHAHA MY NUMBER - from modified ecgen, random 40-bit prime
_a = -3 # Original
_b = 911333413149 # BWAHAHA MY NUMBER - from modified ecgen, random 40-bit integer
_r = 989291303419  # from modified ecgen 
_Gx = 146885098810 # from modified ecgen
_Gy = 687327530143 # from modified ecgen

curve_mini = CurveFp(_p, _a, _b)
generator_mini = Point(curve_mini, _Gx, _Gy, _r)


# AACS curve
# downloaded from https://www.aacsla.com/specifications/AACS_Spec_Common_Final_0952.pdf
_p = 900812823637587646514106462588455890498729007071
_r = 900812823637587646514106555566573588779770753047
_b = 366394034647231750324370400222002566844354703832
_Gx = 264865613959729647018113670854605162895977008838
_Gy = 51841075954883162510413392745168936296187808697

curve_AACS = CurveFp(_p, -3, _b)
generator_AACS = Point(curve_AACS, _Gx, _Gy, _r)

# NIST Curve P-192:
_p = 6277101735386680763835789423207666416083908700390324961279
_r = 6277101735386680763835789423176059013767194773182842284081
# s = 0x3045ae6fc8422f64ed579528d38120eae12196d5L
# c = 0x3099d2bbbfcb2538542dcd5fb078b6ef5f3d6fe2c745de65L
_b = 0x64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1
_Gx = 0x188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012
_Gy = 0x07192b95ffc8da78631011ed6b24cdd573f977a11e794811

curve_192 = CurveFp(_p, -3, _b)
generator_192 = Point(curve_192, _Gx, _Gy, _r)


# NIST Curve P-224:
_p = 26959946667150639794667015087019630673557916260026308143510066298881
_r = 26959946667150639794667015087019625940457807714424391721682722368061
# s = 0xbd71344799d5c7fcdc45b59fa3b9ab8f6a948bc5L
# c = 0x5b056c7e11dd68f40469ee7f3c7a7d74f7d121116506d031218291fbL
_b = 0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4
_Gx = 0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21
_Gy = 0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34

curve_224 = CurveFp(_p, -3, _b)
generator_224 = Point(curve_224, _Gx, _Gy, _r)

# NIST Curve P-256:
_p = 115792089210356248762697446949407573530086143415290314195533631308867097853951
_r = 115792089210356248762697446949407573529996955224135760342422259061068512044369
# s = 0xc49d360886e704936a6678e1139d26b7819f7e90L
# c = 0x7efba1662985be9403cb055c75d4f7e0ce8d84a9c5114abcaf3177680104fa0dL
_b = 0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b
_Gx = 0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296
_Gy = 0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5

curve_256 = CurveFp(_p, -3, _b)
generator_256 = Point(curve_256, _Gx, _Gy, _r)

# NIST Curve P-384:
_p = 39402006196394479212279040100143613805079739270465446667948293404245721771496870329047266088258938001861606973112319
_r = 39402006196394479212279040100143613805079739270465446667946905279627659399113263569398956308152294913554433653942643
# s = 0xa335926aa319a27a1d00896a6773a4827acdac73L
# c = 0x79d1e655f868f02fff48dcdee14151ddb80643c1406d0ca10dfe6fc52009540a495e8042ea5f744f6e184667cc722483L
_b = 0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef
_Gx = 0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7
_Gy = 0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f

curve_384 = CurveFp(_p, -3, _b)
generator_384 = Point(curve_384, _Gx, _Gy, _r)

# NIST Curve P-521:
_p = 6864797660130609714981900799081393217269435300143305409394463459185543183397656052122559640661454554977296311391480858037121987999716643812574028291115057151
_r = 6864797660130609714981900799081393217269435300143305409394463459185543183397655394245057746333217197532963996371363321113864768612440380340372808892707005449
# s = 0xd09e8800291cb85396cc6717393284aaa0da64baL
# c = 0x0b48bfa5f420a34949539d2bdfc264eeeeb077688e44fbf0ad8f6d0edb37bd6b533281000518e19f1b9ffbe0fe9ed8a3c2200b8f875e523868c70c1e5bf55bad637L
_b = 0x051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00
_Gx = 0xc6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66
_Gy = 0x11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650

curve_521 = CurveFp(_p, -3, _b)
generator_521 = Point(curve_521, _Gx, _Gy, _r)

# Certicom secp256-k1
_a = 0x0000000000000000000000000000000000000000000000000000000000000000
_b = 0x0000000000000000000000000000000000000000000000000000000000000007
_p = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
_Gx = 0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
_Gy = 0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8
_r = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141

curve_secp256k1 = CurveFp(_p, _a, _b)
generator_secp256k1 = Point(curve_secp256k1, _Gx, _Gy, _r)

salt_len = 64


def compute_hash( text ):
    text = text.lower()
    #
    # Get the salt bytes, predetermined fixed-size list of integers
    #
    salt = [0] * salt_len
    with open('salt_key.txt', 'r') as filehandle:
        data = filehandle.readlines()
    if( len(data)!=salt_len ):
        print("ERROR: salt code is wrong length, "+str(len(data))+" should be 64")
    for i in range(0,salt_len): # iterate 0..(salt list length-1)
        if( len(data)>i ):
            salt[i] = int(data[i]) % int(2147483647) # clip to 32-bit integer
        else: salt[i] = 0  # should never happen
    #
    # Compute the mash, text + salt compressed into fixed-sized list
    # array of integers, fixed size equal to length of salt list
    # text byte added to each salt value, loop around if too long
    #
    mash = [0] * salt_len
    text = bytearray(text,'utf-8');
    scan_len = max( len(text), salt_len );
    for i in range(0,scan_len): # iterate 0..(salt list length-1)
        j = i % 64
        if( i<salt_len ):
            mash[j] = int(salt[j])
        if( i<len(text) ):
            mash[j] = int(mash[j]) + int(text[i])
        mash[j] = int(mash[j]) % int(2147483647) # clip to 32-bit integer
    #
    # Compute the mash bytes
    # bytearray of the mash integers, four bytes per salt list value
    # ugh, manually convert hash to bytes
    #
    mash_bytes = bytearray(salt_len*4);
    for i in range(0,salt_len): # iterate 0..(salt list length-1)
        j = i*4
        mash_bytes[j+0] = ((mash[i] & 0x000000FF));
        mash_bytes[j+1] = ((mash[i] & 0x0000FF00) >> 8);
        mash_bytes[j+2] = ((mash[i] & 0x00FF0000) >> 16);
        mash_bytes[j+3] = ((mash[i] & 0xFF000000) >> 24);
    #
    # Compute the hash
    # Finally!
    #
    hash_bytes = bytearray( hashlib.sha256( mash_bytes ).digest() )
    #
    # Compute the hash integer
    # integer with the first forty bits of the hash value
    # ugh, manually convert hash bytes to 40-bit integer
    #
    hash = 0;
    hash += int(hash_bytes[0]);
    hash += int(hash_bytes[1]) << 8;
    hash += int(hash_bytes[2]) << 16;
    hash += int(hash_bytes[3]) << 24;
    hash += int(hash_bytes[4]) << 32;
    hash = hash % 0xFFFFFFFFFF; # 40-bit hash
    #
    return hash

# ---------- ---------- 
# MAIN FUNCTION
# ---------- ----------

import sys
import hashlib
from random import SystemRandom, randrange

print('\n'.join('{}: [{}]'.format(*k) for k in enumerate(sys.argv)))

print_usage = False
if( len(sys.argv)<2  ):
    print_usage = True

elif( (sys.argv[1]=='gen') and (len(sys.argv)==2) ):
    print("generating keys...");
    randrange = SystemRandom().randrange
    g = generator_mini
    n = g.order()
    secret = randrange( 1, n )
    pubkey = Public_key( g, g * secret )
    privkey = Private_key( pubkey, secret )
    #print( "public key: "+format(pubkey.point.x(),'02x')+":"+format(pubkey.point.y(),'02x') );
    #print( "secret key: "+format(privkey.secret_multiplier,'02x') );
    with open('secret_key.txt', 'w') as filehandle:  
        filehandle.write( format(privkey.secret_multiplier) )
        filehandle.write('\n')
        filehandle.write( format(pubkey.point.x()) )
        filehandle.write('\n')
        filehandle.write( format(pubkey.point.y()) )
        filehandle.write('\n')
    with open('public_key.txt', 'w') as filehandle:  
        filehandle.write( format(pubkey.point.x()) )
        filehandle.write('\n')
        filehandle.write( format(pubkey.point.y()) )
        filehandle.write('\n')
    with open('salt_key.txt', 'w') as filehandle:
        for i in range(0,salt_len): # iterate 0..(salt code length-1)
            filehandle.write( format(randrange( 1, 2147483647 )) )
            filehandle.write('\n')
    print("done.");

elif( (sys.argv[1]=='sign') and (len(sys.argv)==3) ):
    with open('secret_key.txt', 'r') as filehandle:
        data = filehandle.readlines()
    #
    g = generator_mini
    n = g.order()
    secret = int(data[0])
    pointx = int(data[1])
    pointy = int(data[2])
    keypoint = Point( curve_mini, pointx, pointy, n )
    pubkey = Public_key( g, keypoint )
    privkey = Private_key( pubkey, secret )
    #print( "public key: "+format(pubkey.point.x(),'02x')+":"+format(pubkey.point.y(),'02x') );
    #print( "secret key: "+format(privkey.secret_multiplier,'02x') );
    #
    hash = compute_hash( sys.argv[2] )
    #
    signature = privkey.sign( hash, randrange( 1, n ) )
    #print( "hash: " + str(hash) )
    #print( "serial number: " + format(signature.r,'02x') + '-' + format(signature.s,'02x') )
    print( format(signature.r,'02x') + '-' + format(signature.s,'02x') )

elif( (sys.argv[1]=='open') and (len(sys.argv)==4) ):
    signature_items = sys.argv[2].split("-")
    if( len(signature_items) != 2 ):
        print( "ERROR: expected serial number in format 0123456789-0123456789" );
    else:
        signature_1 = int(signature_items[0],16) # base 16 hexadecimal
        signature_2 = int(signature_items[1],16) # base 16 hexadecimal
        signature = Signature( signature_1, signature_2 )
        print( "serial number: "+format(signature.r,'02x')+"-"+format(signature.s,'02x') )
        #
        with open('public_key.txt', 'r') as filehandle:
            data = filehandle.readlines()
        #	
        g = generator_mini
        n = g.order()
        pointx = int(data[0])
        pointy = int(data[1])
        keypoint = Point( curve_mini, pointx, pointy, n )
        pubkey = Public_key( g, keypoint )
        print( "public key: "+format(pubkey.point.x(),'02x')+":"+format(pubkey.point.y(),'02x') );
        #
        hash = compute_hash( sys.argv[3] )
        print( "hash: "+str(hash) )
        #
        if pubkey.verifies( hash, signature ):
            print("SUCCESSFULLY OPENED - CORRECT SIGNATURE")
        else:
            print("FAIL TO OPEN - WRONG SIGNATURE")

elif( sys.argv[1]=='test' ):
    g = generator_mini
    n = g.order()
    #
    secret = randrange( 1, n )
    pubkey = Public_key( g, g * secret )
    privkey = Private_key( pubkey, secret )
    # Signing a hash value:
    hash = randrange( 1, n )
    signature = privkey.sign( hash, randrange( 1, n ) )
    #
    # Verifying a signature for a hash value:
    if pubkey.verifies( hash, signature ):
        print("Demo verification succeeded.")
    else:
        print("*** Demo verification failed.")
    # Verification fails if the hash value is modified:
    #if pubkey.verifies( hash-1, signature ):
    tampered = False
    tampered_attempts = 10000000
    for i in range(0,tampered_attempts):
        signature_test = Signature( randrange(0,n), randrange(0,n) )
        if pubkey.verifies( hash, signature_test ):
            print("**** Demo verification failed to reject tampered hash after "+str(i)+"tries.")
            tampered = True
        if (i%1000) == 0:
            print("hash = "+str(hash))
            print("most recent attempt = ("+format(signature_test.s,'02x')+","+format(signature_test.r,'02x')+") " )
            print("attempts so far: "+str(i))
            hash = randrange( 1, n )
    #tampered_attempts = 500
    #for i in range(0,tampered_attempts):
    #    if pubkey.verifies( randrange(0,0xFFFFFFFFFF), signature ):
    #        print("**** Demo verification failed to reject tampered hash after "+str(i)+"tries.")
    #        tampered = True
    #
    if not tampered:
        print("Demo verification correctly rejected tampered hash after "+str(tampered_attempts)+" tries.")
else:
    print_usage = True

	
if( print_usage ):
    print( "" )
    print( " usage: " )
    print( " python ECDSA_Python.py [gen | sign FIRSTNAME&LASTNAME&EMAIL | open SIG TEXT]")
    print( "" )
    print( " gen: writes secret_key.txt and public_key.txt text files,")
    print( " don't lose these, and only generate new keys once per software release")
    print( "" )
    print( " sign: pass utf8 FIRSTNAME, LASTNAME, and EMAIL, chars \"\& converted to _")
    print( " and concatenate the three values together with & characters separating them")
    print( "" )
    print( " open: pass signature value from sign command, and same text from sign command")
    print( " prints whether signature matches text")
    print( "" )

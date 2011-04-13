// ----------------------------------------------------------------------------
// Project: library
/// @file   extraint.h
/// @author Andy Parkins
//
// Version Control
//    $Author$
//      $Date$
//        $Id$
//
// Legal
//    Copyright 2011  Andy Parkins
//
// ----------------------------------------------------------------------------

// Catch multiple includes
#ifndef EXTRAINT_H
#define EXTRAINT_H

// -------------- Includes
// --- C
// --- C++
#include <vector>
#include <string>
#include <stdexcept>
// --- Qt
// --- OS
// --- Project
// --- Project lib


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TGenericBigInteger
// Description:
/// Arbitrary precision integer.
//
/// Got some help by reading the source for this similar implementation,
/// which wasn't written how I like, but gave me hints to the
/// appropriate algorithms.
///
/// https://mattmccutchen.net/bigint/
//
template <typename tLittleInteger>
class TGenericBigInteger
{
  public:
	enum eComparisonResult {
		LessThan = -1,
		EqualTo = 0,
		GreaterThan = 1
	};
	typedef vector<tLittleInteger> tLittleDigitsVector;
	typedef typename tLittleDigitsVector::size_type tIndex;
	static const size_t bitsPerBlock;

  public:
	TGenericBigInteger() { LittleDigits.push_back(0); }
	TGenericBigInteger( const TGenericBigInteger &O ) { operator=(O); }
	TGenericBigInteger( const string &s ) { operator=(s); }
	TGenericBigInteger( int t ) { operator=(static_cast<unsigned int>(t) ); }
	TGenericBigInteger( tLittleInteger r0 ) { operator=(r0); }
	TGenericBigInteger( tLittleInteger r1, tLittleInteger r0 );
	TGenericBigInteger( tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 );
	TGenericBigInteger( tLittleInteger r3, tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 );
	TGenericBigInteger( unsigned long long t ) { operator=(t); }

	void invalidate() { LittleDigits.clear(); }
	bool isValid() const { return !LittleDigits.empty(); }
	bool isZero() const { return LittleDigits.size() == 1 && LittleDigits.back() == 0; }

	// Assignment
	TGenericBigInteger &operator=( const string & );
//	TGenericBigInteger &operator=( unsigned int t ) { LittleDigits.clear(); LittleDigits.push_back(t); return *this; }
	TGenericBigInteger &operator=( unsigned long long t );
	TGenericBigInteger &operator=( const TGenericBigInteger &O ) { LittleDigits = O.LittleDigits; return *this; }

	// Access
	bool getBit( tIndex bi ) const {
		if( bi/bitsPerBlock < LittleDigits.size() ) {
			return (LittleDigits[bi/bitsPerBlock] & (1 << (bi % bitsPerBlock))) != 0;
		} else {
			return false;
		}
	}
	tLittleInteger getBlock( tIndex bl ) const { return isValid() ? LittleDigits[bl] : 0; }
	tIndex highestBit() const;
	tIndex lowestBit() const;
//	bool setBit( tIndex bi, bool b );

	// Arithmetic
	TGenericBigInteger operator+( const TGenericBigInteger & ) const;
	TGenericBigInteger operator-( const TGenericBigInteger & ) const;
	TGenericBigInteger operator*( const TGenericBigInteger & ) const;
	TGenericBigInteger operator/( const TGenericBigInteger & ) const;
	TGenericBigInteger operator%( const TGenericBigInteger & ) const;
	TGenericBigInteger operator&( const TGenericBigInteger & ) const;
	TGenericBigInteger operator|( const TGenericBigInteger & ) const;
	TGenericBigInteger operator^( const TGenericBigInteger & ) const;
	TGenericBigInteger operator~() const;
	TGenericBigInteger operator<<( tIndex ) const;
	TGenericBigInteger operator>>( tIndex ) const;

	TGenericBigInteger &operator +=(const TGenericBigInteger &x);
	TGenericBigInteger &operator -=(const TGenericBigInteger &x);
	TGenericBigInteger &operator *=(const TGenericBigInteger &x);
	TGenericBigInteger &operator /=(const TGenericBigInteger &x);
	TGenericBigInteger &operator %=(const TGenericBigInteger &x);
	TGenericBigInteger &operator &=(const TGenericBigInteger &x);
	TGenericBigInteger &operator |=(const TGenericBigInteger &x);
	TGenericBigInteger &operator ^=(const TGenericBigInteger &x);
	TGenericBigInteger &operator <<=( tIndex );
	TGenericBigInteger &operator >>=( tIndex );

	TGenericBigInteger &operator++();
	TGenericBigInteger &operator++( int );
	TGenericBigInteger &operator--();
	TGenericBigInteger &operator--( int );

	// Comparison
	eComparisonResult compareTo( const TGenericBigInteger & ) const;
	bool operator<( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && compareTo(O) == LessThan; }
	bool operator>( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && compareTo(O) == GreaterThan; }
	bool operator==( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && compareTo(O) == EqualTo; }
	bool operator!=( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && !(*this == O); }
	bool operator<=( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && !(*this > O); }
	bool operator>=( const TGenericBigInteger &O ) const { return isValid() && O.isValid() && !(*this < O); }

	virtual ostream &printOn( ostream & ) const;

  protected:
	void normalise();

	template <typename T> T convertToSignedPrimitive() const;
	template <typename T> T convertToPrimitive() const;

	void add(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void subtract(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void multiply(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void bitAND(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void bitOR(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void bitXOR(const TGenericBigInteger &a, const TGenericBigInteger &b);
	void bitNOT(const TGenericBigInteger &a);
	void bitShiftLeft(const TGenericBigInteger &a, tIndex b);
	void bitShiftRight(const TGenericBigInteger &a, tIndex b);
	void divideWithRemainder(const TGenericBigInteger &b, TGenericBigInteger &q);

	static void blockMultiply(tLittleInteger &R1, tLittleInteger &R0,
		tLittleInteger B, const tLittleInteger C );

	void blockShiftLeft(const TGenericBigInteger &a, tIndex b);
	void blockShiftRight(const TGenericBigInteger &a, tIndex b);

  protected:
	tLittleDigitsVector LittleDigits;
};


// -------------- Constants


// -------------- Inline Functions

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator+(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.add(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator-(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.subtract(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator*(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.multiply(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator/(const TGenericBigInteger &x) const
{
	if( x.isZero() )
		throw runtime_error("TGenericBigInteger<tLittleInteger>::operator/: division by zero");
	TGenericBigInteger q, r;
	r = *this;
	r.divideWithRemainder(x, q);
	return q;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator%(const TGenericBigInteger &x) const
{
	if( x.isZero() )
		throw runtime_error("TGenericBigInteger<tLittleInteger>::operator/: division by zero");
	TGenericBigInteger q, r;
	r = *this;
	r.divideWithRemainder(x, q);
	return r;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator&(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.bitAND(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator|(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.bitOR(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator^(const TGenericBigInteger &x) const
{
	TGenericBigInteger ans;
	ans.bitXOR(*this, x);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator~() const
{
	TGenericBigInteger ans;
	ans.bitNOT(*this);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator<<(tIndex b) const
{
	TGenericBigInteger ans;
	ans.bitShiftLeft(*this, b);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator>>(tIndex b) const
{
	TGenericBigInteger ans;
	ans.bitShiftRight(*this, b);
	return ans;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator+=(const TGenericBigInteger &x)
{
	add(*this, x);
	return *this;
}
template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator-=(const TGenericBigInteger &x)
{
	subtract(*this, x);
	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator*=(const TGenericBigInteger &x)
{
	multiply(*this, x);
	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator/=(const TGenericBigInteger &x)
{
	if( x.isZero() )
		throw runtime_error("TGenericBigInteger<tLittleInteger>::operator/: division by zero");
	/* The following technique is slightly faster than copying *this first
	 * when x is large. */
	TGenericBigInteger q;
	divideWithRemainder(x, q);
	// *this contains the remainder, but we overwrite it with the quotient.
	*this = q;

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator%=(const TGenericBigInteger &x)
{
	if( x.isZero() )
		throw runtime_error("TGenericBigInteger<tLittleInteger>::operator/: division by zero");
	TGenericBigInteger q;
	// Mods *this by x.  Don't care about quotient left in q.
	divideWithRemainder(x, q);

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator&=(const TGenericBigInteger &x)
{
	bitAND(*this, x);

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator|=(const TGenericBigInteger &x)
{
	bitOR(*this, x);

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator^=(const TGenericBigInteger &x)
{
	bitXOR(*this, x);

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator<<=(tIndex b)
{
	bitShiftLeft(*this, b);

	return *this;
}

template <typename tLittleInteger>
inline TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator>>=(tIndex b)
{
	bitShiftRight(*this, b);

	return *this;
}

// ---------

template <typename tLittleInteger>
inline ostream &operator<<( ostream &s, const TGenericBigInteger<tLittleInteger> &O )
{
	return O.printOn(s);
}


// -------------- Function prototypes


// -------------- Template instantiations
typedef TGenericBigInteger<unsigned int> TBigInteger;



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif

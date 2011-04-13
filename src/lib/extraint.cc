// ----------------------------------------------------------------------------
// Project: library
/// @file   extraint.cc
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

// Module include
#include "extraint.h"

// -------------- Includes
// --- C
#include <ctype.h>
// --- C++
#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>
#include <stdexcept>
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations
template class TGenericBigInteger<unsigned int>;


// -------------- Class member definitions

//
// Function:	TGenericBigInteger :: bitsPerBlock
// Description:
//
template <typename tLittleInteger>
const size_t TGenericBigInteger<tLittleInteger>::bitsPerBlock = numeric_limits<tLittleInteger>::digits;

//
// Function:	TGenericBigInteger :: TGenericBigInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger>::TGenericBigInteger( tLittleInteger r1, tLittleInteger r0 )
{
	LittleDigits.push_back(r0);
	LittleDigits.push_back(r1);
	normalise();
}

//
// Function:	TGenericBigInteger :: TGenericBigInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger>::TGenericBigInteger( tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 )
{
	LittleDigits.push_back(r0);
	LittleDigits.push_back(r1);
	LittleDigits.push_back(r2);
	normalise();
}

//
// Function:	TGenericBigInteger :: TGenericBigInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger>::TGenericBigInteger( tLittleInteger r3, tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 )
{
	LittleDigits.push_back(r0);
	LittleDigits.push_back(r1);
	LittleDigits.push_back(r2);
	LittleDigits.push_back(r3);
	normalise();
}

//
// Function:	TGenericBigInteger :: operator=
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &TGenericBigInteger<tLittleInteger>::operator=( const string &s )
{
	static const unsigned int Base = 10;
	string::size_type pos;

	// Start at zero
	(*this) = 0;

	pos = s.size();
	while( pos > 0 ) {
		pos--;

		if( !isdigit(s[pos]) )
			break;
		(*this) += static_cast<unsigned int>(s[pos] - '0');

		// Shift left by base ready for next round
		if( pos > 0 )
			(*this) *= Base;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator=
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &TGenericBigInteger<tLittleInteger>::operator=( unsigned long long t )
{
	LittleDigits.clear();

	do {
		LittleDigits.push_back( t & numeric_limits<tLittleInteger>::max() );
		t >>= numeric_limits<tLittleInteger>::digits;
	} while( t > 0 );

	return *this;
}

//
// Function:	TGenericBigInteger :: highestBit
// Description:
//
template <typename tLittleInteger>
typename TGenericBigInteger<tLittleInteger>::tIndex
TGenericBigInteger<tLittleInteger>::highestBit() const
{
	if( !isValid() || isZero() )
		return 0;

	tIndex b = (LittleDigits.size() - 1) * bitsPerBlock;
	tLittleInteger x;

	// We have the whole block number of bits, now we just need the
	// highest one bit in the highest block

	x = LittleDigits.back();
	while( x > 0 ) {
		b++;
		x >>= 1;
	}

	return b-1;
}

//
// Function:	TGenericBigInteger :: lowestBit
// Description:
//
template <typename tLittleInteger>
typename TGenericBigInteger<tLittleInteger>::tIndex
TGenericBigInteger<tLittleInteger>::lowestBit() const
{
	if( !isValid() || isZero() )
		return 0;

	tIndex b = 0;

	// Find the first non-zero block
	while( LittleDigits[b] == 0 ) {
		b++;
	}

	tLittleInteger x = LittleDigits[b];

	// Convert from block to bits
	b = b * bitsPerBlock;

	// We have the whole block number of bits, now we just need the
	// highest one bit in the highest block

	while( (x & 1) == 0 && x != 0 ) {
		b++;
		x >>= 1;
	}

	return b;
}

//
// Function:	TGenericBigInteger :: compareTo
// Description:
//
template <typename tLittleInteger>
typename TGenericBigInteger<tLittleInteger>::eComparisonResult
TGenericBigInteger<tLittleInteger>::compareTo( const TGenericBigInteger<tLittleInteger> &C ) const
{
	// Size will give us a quick anser
	if( LittleDigits.size() < C.LittleDigits.size() ) {
		return LessThan;
	} else if( LittleDigits.size() > C.LittleDigits.size() ) {
		return GreaterThan;
	}

	// Equal sizes, so we must compare element by element

	typename tLittleDigitsVector::const_reverse_iterator it1, it2;

	// Start two iterators at the end (being the most significant
	// block)
	it1 = LittleDigits.rbegin();
	it2 = C.LittleDigits.rbegin();

	while( it1 != LittleDigits.rend() ) {
		if( (*it1) < (*it2) ) {
			return LessThan;
		} else if( (*it1) > (*it2) ) {
			return GreaterThan;
		}

		it1++;
		it2++;
	}

	// Equal sizes, equal content
	return EqualTo;
}

//
// Function:	TGenericBigInteger :: normalise
// Description:
/// Remove leading zeroes.
//
/// Remove leading zeroes (remembering that the digits array is stored
/// least significant first, and so the leading zeroes are at the end)
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::normalise()
{
	while( LittleDigits.size() > 1 && LittleDigits.back() == 0 )
		LittleDigits.pop_back();
}

//
// Macro:	TGEN_BIG_INT_SELF_CHECK
// Description:
/// Check for "this" as one of arguments
//
// The mathematical operators will fail if they operate directly on
// themselves.  This macro detects a self operation and reruns the
// operation using a temporary copy.
//
#define TGEN_BIG_INT_SELF_CHECK( ArgumentIsSelf, Operation ) \
	if( ArgumentIsSelf ) { \
		TGenericBigInteger<tLittleInteger> Temporary; \
		Temporary.Operation; \
		*this = Temporary; \
		return; \
	}

//
// Function:	TGenericBigInteger :: operator+=
// Description:
/// Implement A = A + B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator+=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	// If one is invalid, then the answer is invalid
	if( !isValid() || !B.isValid() ) {
		invalidate();
		return *this;
	}

	// Starting at the least significant blocks (the front of the
	// vector), we now iterate through until we've used up the shortest
	// input argument.
	bool carryIn = false, carryOut;
	tLittleInteger temp;

	// Point at begining of both arrays
	itA = LittleDigits.begin();
	itB = B.LittleDigits.begin();

	while( itA != LittleDigits.end() && itB != B.LittleDigits.end() ) {
		// Perform sum
		temp = *itA + *itB;

		// A carry has ocurred if we've got an answer smaller than the
		// inputs.  The result will be the wrap around equivalent of the
		// answer, so we must note that we need an extra in the next
		// block if we detect it.
		// http://www.fefe.de/intof.html
		carryOut = (temp < *itA);

		// Was there a carry from the previous block?
		if( carryIn ) {
			// Add it in
			temp++;
			// Adding a carry can overflow too; check for it.  If we've
			// already had a carry, then we leave it set.
			carryOut |= (temp == 0);
		}

		// Store
		*itA = temp;
		carryIn = carryOut;

		// Next element
		itA++;
		itB++;

	}
	// Local array still has digits in it
	while( itA != LittleDigits.end() && carryIn ) {
		if( carryIn )
			(*itA)++;
		if( *itA != 0 )
			carryIn = false;
		itA++;
	}
	// Remote array still has digits, whilst we have run out. Basically
	// copy with only the carry to worry about
	while( itB != B.LittleDigits.end() ) {
		LittleDigits.push_back( *itB );
		if( carryIn )
			LittleDigits.back()++;
		if( LittleDigits.back() != 0 )
			carryIn = false;
		itB++;
	}

	// If there is _still_ a carry remaining, we'll need the extra block
	// (the one we reserved in our LittleDigits.reserve() call ealier
	if( carryIn )
		LittleDigits.push_back(1);

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator-=
// Description:
/// Implement A = A - B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator-=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	// If one is invalid, then the answer is invalid
	if( !isValid() || ! B.isValid() ) {
		invalidate();
		return *this;
	}

	if( LittleDigits.size() < B.LittleDigits.size()) {
		// If a is shorter than b, the result is negative.
		throw runtime_error("TGenericBigInteger<tLittleInteger>::operator-=: "
			"Negative result in unsigned calculation");
	}

	// ---

	bool borrowIn = false, borrowOut;

	// Point at begining of both arrays
	itA = LittleDigits.begin();
	itB = B.LittleDigits.begin();

	// For each block index that is present in both inputs...
	while( itA != LittleDigits.end() && itB != B.LittleDigits.end() ) {
		// If we're short in this block; we must borrow from the next
		// block; we're "short" when the subtraction is going to be
		// negative -- which means wrapped around
		borrowOut = (*itB > *itA);

		// Perform subtraction
		*itA -= *itB;

		// Check for a borrow request from the previous loop
		if( borrowIn ) {
			// If we need to borrow but our block is already zero, then
			// we'll have to borrow from the next block.  If we're
			// already borrowing then it will take care of itself, we've
			// got a whole digits worth of extra numbers already
			borrowOut |= (*itA == 0);
			// Apply our borrow
			(*itA)--;
		}

		// Our borrowOut, is the next loop's borrowIn
		borrowIn = borrowOut;

		// Next element
		itA++;
		itB++;
	}

	// It's impossible to run out of A before B, so we must be here
	// because we've run out of B digits; therefore we're only concerned
	// with applying any remaining carrys to A
	while( itA != LittleDigits.end() && borrowIn ) {
		// If there is a borrow remaining, then apply it
		if( borrowIn ) {
			// If the current digit is already zero, we're going to need
			// to borrow from the next position
			borrowIn = (*itA == 0);
			(*itA)--;
		}
		itA++;
	}

	// If there is still a borrow, then the result would be negative,
	// which we checked for at the top, and so should be impossible
	// here.
	if( borrowIn )
		throw logic_error("TGenericBigInteger<tLittleInteger>::operator-=: Negative result in unsigned calculation impossible here");

	// We could quite easily have made leading zero-containing blocks.
	// Remove them.
	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator*=
// Description:
/// Implement A = A * B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator*=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	// We can't do this multiply in place, as we need an accumulator
	TGenericBigInteger<tLittleInteger> C(*this);

	// Warn our own vector how much space it's going to need
	LittleDigits.clear();
	LittleDigits.assign( B.LittleDigits.size() + C.LittleDigits.size(), 0 );

	// We're multiplying two N digit numbers.  The digits are in base
	// 2^32 (say).  We multiply each digit pair and add to an
	// accumulator.  Let's say we're multiplying two two digit numbers,
	// the result of each product of two single digits is two digits
	//
	//  B = 51;  C = 69
	//
	//       [0] [1]           [0] [1]           [0] [1]
	//   [0] 1*9 1*6   ->  [0]  09  06  and  [0]   0   1
	//   [1] 5*9 5*6       [1]  45  30       [1]   1   2
	//                         result           position
	// Using the position table to shift left the result table:
	//
	//          2 1 0
	//        -------
	// [0][0]       9
	// [0][1]     6
	// [1][0]   4 5
	// [1][1] 3 0
	//        -------
	//        3 5 1 9

	tLittleInteger R0, R1;
	TGenericBigInteger R;

	for( tIndex i = 0; i < B.LittleDigits.size(); i++ ) {
		for( tIndex j = 0; j < C.LittleDigits.size(); j++ ) {
			blockMultiply( R1, R0, B.LittleDigits[i], C.LittleDigits[j] );

			// Use TGenericBigInteger to do the digit shift and addition with
			// carry
			R.blockShiftLeft( TGenericBigInteger<tLittleInteger>(R1,R0), i+j );
			(*this) += R;
		}
	}

	return *this;
}

//
// Function:	TGenericBigInteger :: blockMultiply
// Description:
/// Single block multiply
//
/// Calculate [R1,R0] = (B * C)
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::blockMultiply(tLittleInteger &R1, tLittleInteger &R0,
		tLittleInteger B, const tLittleInteger C )
{
	static const size_t halfBlock = bitsPerBlock >> 1;
	static const tLittleInteger highMask = static_cast<tLittleInteger>(-1) << halfBlock;
	static const tLittleInteger lowMask = ~highMask;
	tLittleInteger b0, b1, c0, c1;
	tLittleInteger W, X, Y, Z;
	tLittleInteger Carry = 0;

	// We're going to do long multiplication, using X bit width digits.
	// We won't make any assumptions about the width of tLittleInteger,
	// other than it is divisible by two.  In particular we won't assume
	// that there is another type that is twice as wide (even though
	// it's possible that there is).
	//
	// As an example, let's pretend that tLittleInteger is 32 bits wide.
	// That means the result would be 64 bits wide.  We can't assume
	// that there is a 64 bit wide data type.  Instead we convert our
	// two 32 bit numbers into two numbers with two 16-bit wide digits.

	// e.g.
	//      32-bits    * 32-bits     = 64-bits
	//      0xffffdddd * 0xeeeecccc  = 0xeeeeacef9e26e81c
	//
	// Don't think of these as base 16 numbers, think of them as base
	// 2^32 numbers.  Now, let's represent these base 2^32 numbers in
	// base 2^16
	//
	//       65536s   1s   65536s   1s
	//         ffff dddd *   eeee cccc

	b1 = (B & highMask) >> halfBlock;
	b0 = (B & lowMask);
	c1 = (C & highMask) >> halfBlock;
	c0 = (C & lowMask);

	// Now simply follow the method we would use for paper long
	// multiplication.  We get four 32-bit values as output, which we'll
	// call W, X, Y, and Z.
	//
	//     W = ffff * eeee = EEED1112
	//     X = dddd * eeee =     CF11B976
	//     Y = ffff * cccc =     CCCB3334
	//     Z = dddd * cccc =         B17CE81C

	W = b1 * c1;
	X = b0 * c1;
	Y = b1 * c0;
	Z = b0 * c0;

	// Then we sum the appropriate words to make our two output 32bit
	// values, which we'll call R1, and R0.
	//     R0 = Z
	//        + (Y & 0xffff) << 16
	//        + (X & 0xffff) << 16
	//     R1 = W
	//        + (X & 0xffff0000) >> 16
	//        + (Y & 0xffff0000) >> 16
	//
	// We have the small problem of the overflow in the calculation of
	// R0, so we have to do our additions in stages.
	//
	//  R0 = Z
	//  R0 = R0 + ((Y & 0xffff) << 16)  and  O1 = overflow
	//  R0 = R0 + ((X & 0xffff) << 16)  and  O1 = O1 + overflow

	// XXX: "gcc as of version 4.1 abuses this and optimizes away checks
	// that would only be true if there was an overflow, in particular
	// checks that check for an overflow." http://www.fefe.de/intof.html

	R0 = Z + (Y << halfBlock);
	if( R0 < Z )
		Carry++;
	R0 += (X << halfBlock);
	if( R0 < (X<<halfBlock) )
		Carry++;

	//  R1 = W
	//  R1 = R1 + ((X & 0xffff0000) >> 16)
	//  R1 = R1 + ((Y & 0xffff0000) >> 16)
	//  R1 = R1 + O1

	R1 = W
		+ ((X & highMask) >> halfBlock)
		+ ((Y & highMask) >> halfBlock)
		+ Carry;
}

//
// Function:	TGenericBigInteger :: divideWithRemainder
// Description:
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::divideWithRemainder(const TGenericBigInteger<tLittleInteger> &d, TGenericBigInteger<tLittleInteger> &Q)
{
	TGEN_BIG_INT_SELF_CHECK( this == &d || this == &Q, divideWithRemainder(d,Q) );

	// "this" is the numerator; D is the denominator, and we'll be
	// storing the remainder in this and the quotient in Q.

	TBigInteger D(d);

	// Binary division goes like this:
	//
	//   11 into 11011
	//   11 into 11     goes 1 r 0
	//   11 into  00    goes 0 r 0
	//   11 into   01   goes 0 r 1 (the remainder carries down)
	//   11 into    11  goes 1 r 0
	//
	// Obviously, this is going to be horrendously inefficient with our
	// arbitrary precision numbers, but it's the best we can do, as
	// there is no way to build up a division out of smaller components.

	// Note however that we only need to process the ones in the
	// dividend, zeroes are guaranteed to return zero.

	// Note also that we're really saying this:
	//
	//  11 into 11111
	//  11000 into 11111  goes 1 times remainder 111
	//   1100 into   111  goes 0 times remainder 111
	//    110 into   111  goes 1 times remainder   1
	//     11 into     1  goes 0 times remainder   1
	//
	// In other words, we shift left the divisor to make it the same
	// length as the dividend, then we see if it will subtract.  If it
	// does, then we push 1 into the quotient, and pass the remainder to
	// the next stage.  If it does not, we push a 0 into the quotient.
	// Then we shift right the dividend and try again.  Being binary, it
	// will either subtract once or not at all, making the decision
	// easy.  When we have undone all the shifts we initially
	// introduced, then whatever remains is the remainder.

	tIndex denominatorHighBit = D.highestBit();
	tIndex numeratorHighBit = highestBit();

	if( denominatorHighBit > numeratorHighBit ) {
		// The numerator is the remainder and the quotient is zero
		// because the denominator is larger than the numerator
		Q = 0;
		return;
	}

//	cerr << "NHB - DHB = " << numeratorHighBit << " - " << denominatorHighBit << endl;
//	cerr << "Numerator = " << *this << endl;
//	cerr << "Denominator    = " << D << endl;

	// First we shift the denominator up to have its high bit in the
	// same place as the numerator high bit
	D <<= numeratorHighBit - denominatorHighBit;

	tIndex count = numeratorHighBit - denominatorHighBit + 1;

//	cerr << "Denominator[0] = " << D << endl;
//	cerr << "count = " << count << endl;

	while( count > 0 ) {
		count--;
		Q <<= 1;

		// If the current denominator is less than the numerator, then
		// it will divide...
		if( D < *this ) {
			// ... so we push a one
			Q |= 1;
			// and subtract this denominator to leave a remainder for
			// the next cycle
			*this -= D;
		} else {
			// ... if not, then we push a zero, and the previous
			// remainder remains (as we haven't subtracted anything from
			// it)
		}

		// Next denominator
		D >>= 1;
	}
}

//
// Function:	TGenericBigInteger :: operator&=
// Description:
/// Implement A = A & B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator&=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	// Start at least significant end of source
	itA = LittleDigits.begin();
	itB = B.LittleDigits.begin();

	while( itA != LittleDigits.end() && itB != B.LittleDigits.end() ) {
		*itA = *itA & *itB;
		itA++;
		itB++;
	}
	// Implied zeroes can be ignored, since A & 0 is 0, and normalise()
	// would strip them.

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator|=
// Description:
/// Implement A = A | B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator|=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	// Start at least significant end of source
	itA = LittleDigits.begin();
	itB = B.LittleDigits.begin();

	while( itA != LittleDigits.end() && itB != B.LittleDigits.end() ) {
		*itA = *itA | *itB;
		itA++;
		itB++;
	}
	// No need to do implied copy of A, we _are_ A.
	// Implied zeroes are just copies
	while( itB != B.LittleDigits.end() ) {
		LittleDigits.push_back( *itB );
		itB++;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator^=
// Description:
/// Implement A = A ^ B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator^=(
		const TGenericBigInteger<tLittleInteger> &B)
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	// Start at least significant end of source
	itA = LittleDigits.begin();
	itB = B.LittleDigits.begin();

	while( itA != LittleDigits.end() && itB != B.LittleDigits.end() ) {
		*itA = *itA ^ *itB;
		itA++;
		itB++;
	}
	// No need to do implied copy of A, we _are_ A.
	// Implied zeroes are just copies
	while( itB != B.LittleDigits.end() ) {
		LittleDigits.push_back( *itB );
		itB++;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: operator~
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::operator~() const
{
	TGenericBigInteger<tLittleInteger> R;
	typename tLittleDigitsVector::const_iterator itA;

	// Start at least significant end of source
	itA = LittleDigits.begin();

	while( itA != LittleDigits.end() ) {
		R.LittleDigits.push_back( ~(*itA) );
		itA++;
	}

	// NOTE: We can't supply the infinite number of ones, so we'll just
	// leave them off.  It's the users problem to ensure they have the
	// right width.

	R.normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: bitShiftLeft
// Description:
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::bitShiftLeft(const TGenericBigInteger &a, tIndex b)
{
	typename tLittleDigitsVector::iterator it;
	tLittleInteger overflowBits, lastBits = 0;

	TGEN_BIG_INT_SELF_CHECK( this == &a, bitShiftLeft(a,b) );

	// The block part can be done with blockShiftLeft(), we shift by one
	// more than necessary, as we're going to shift the last block
	// downwards and the last block upwards
	blockShiftLeft( a, b / bitsPerBlock );

	// The left over must be done with a bit shift
	b = b % bitsPerBlock;

	if( b == 0 )
		return;

	// Start at least significant end
	for( it = LittleDigits.begin(); it != LittleDigits.end(); it++ ) {
		// Save the overflow bits
		overflowBits = (*it) >> (bitsPerBlock - b);
		// Shift
		(*it) <<= b;
		// Mix in overflow bits from previous block
		(*it) |= lastBits;
		// Send the overflow to the next block
		lastBits = overflowBits;
	}
	if( lastBits != 0 )
		LittleDigits.push_back( lastBits );

	normalise();
}

//
// Function:	TGenericBigInteger :: bitShiftRight
// Description:
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::bitShiftRight(const TGenericBigInteger &a, tIndex b)
{
	typename tLittleDigitsVector::reverse_iterator it;
	tLittleInteger underflowBits, lastBits = 0;

	TGEN_BIG_INT_SELF_CHECK( this == &a, bitShiftRight(a,b) );

	// The block part can be done with blockShiftLeft()
	blockShiftRight( a, b / bitsPerBlock );
	// The left over must be done with a bit shift
	b = b % bitsPerBlock;

	// Start at the most significant end
	for( it = LittleDigits.rbegin(); it != LittleDigits.rend(); it++ ) {
		// We're going to shift right, so some bits are going to drop
		// off the right hand edge.
		//   DDDDxxxx -> 0000DDDD
		// We're going to put the underflow bits into the next block, so
		// we shift them left into our hold, so they are in the right
		// place.
		//   DDDDxxxx -> xxxx0000
		// Which we can then OR into our next block
		underflowBits = *it << (bitsPerBlock - b);
		// Perform the shift for the current block
		*it >>= b;
		// OR in the bits from the previous block
		*it |= lastBits;

		// Send the underflowBits to the next block
		lastBits = underflowBits;
	}
	// Remaining lastBits can be discarded, as they've been pushed off
	// the right hand edge.

	normalise();
}

//
// Function:	TGenericBigInteger :: blockShiftLeft
// Description:
// xxx yyy zzz -> xxx yyy zzz 0
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::blockShiftLeft(const TGenericBigInteger<tLittleInteger> &a, tIndex b)
{
	typename tLittleDigitsVector::const_iterator it;

	TGEN_BIG_INT_SELF_CHECK( this == &a, blockShiftLeft(a,b) );

	LittleDigits.clear();

	// Push extra blocks at the least significant end
	while( b-- > 0 )
		LittleDigits.push_back(0);

	copy( a.LittleDigits.begin(), a.LittleDigits.end(), back_inserter( LittleDigits ) );
}

//
// Function:	TGenericBigInteger :: blockShiftRight
// Description:
// xxx yyy zzz -> xxx yyy
//
template <typename tLittleInteger>
void TGenericBigInteger<tLittleInteger>::blockShiftRight(const TGenericBigInteger<tLittleInteger> &a, tIndex b)
{
	typename tLittleDigitsVector::const_iterator it;

	TGEN_BIG_INT_SELF_CHECK( this == &a, blockShiftRight(a,b) );

	LittleDigits.clear();

	// Start at least significant end of source
	it = a.LittleDigits.begin();

	// Skip b blocks
	it += b;

	// Copy remainder
	copy( it, a.LittleDigits.end(), back_inserter( LittleDigits ) );
}

//
// Function:	TGenericBigInteger :: printOn
// Description:
//
template <typename tLittleInteger>
ostream &TGenericBigInteger<tLittleInteger>::printOn( ostream &s ) const
{
	typename tLittleDigitsVector::const_reverse_iterator it1;

	if( !isValid() ) {
		s << "!INVALID!";
		return s;
	}

	it1 = LittleDigits.rbegin();

	s << hex << "0x";
	while( it1 != LittleDigits.rend() ) {
		// There are four bits per hex digit, so ensure that every block
		// (except the first) shows the appropriate zero padding.
		if( it1 != LittleDigits.rbegin() )
			s << setw( bitsPerBlock/4 ) << setfill('0');
		s << (*it1);
		it1++;
	}
	s << dec;

	return s;
}


// -------------- Function definitions


#ifdef UNITTEST
#include "logstream.h"
#include "extratime.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	static const unsigned int LOOPS = 1000000;

	try {
		//      2^32 = 4294967296
		//      2^64 = 18446744073709551616
		TBigInteger i("99999999999999999999"); // 0x56BC75E2D630FFFFF
		TBigInteger thirtyTwoB( 0xffffdddd ), thirtyTwoC( 0xeeeecccc );
		TBigInteger sixtyFourB( 0xffffffff, 0xdddddddd ),
					sixtyFourC( 0xeeeeeeee, 0xcccccccc );
		TBigInteger j, k;

		j = 1LL;

		log(TLog::Status) << "Testing constructors and initialisation" << endl;
		log() << "i = " << i << endl;
		if( i.getBlock(2) != 0x5 || i.getBlock(1) != 0x6bc75e2d || i.getBlock(0) != 0x630fffff )
			throw logic_error("Assignment from decimal-representing string incorrect");
		log() << "j = " << j << endl;
		log() << "k = " << k << endl;

		log(TLog::Status) << "Testing bit operators on large numbers" << endl;

		k = TBigInteger( 0x1, 0xffffffff ) << 1;
		log() << "k = " << k << endl;
		if( k.getBlock(1) != 0x3 || k.getBlock(0) != 0xfffffffe )
			throw logic_error( "Shift left incorrect (non block multiple, edge)" );
		log() << "k.highestBit() = " << k.highestBit() << "; k.lowestBit() = " << k.lowestBit() << endl;
		if( k.lowestBit() != 1 )
			throw logic_error( "TBigInteger::lowestBit() incorrect" );
		if( k.highestBit() != 33 )
			throw logic_error( "TBigInteger::highestBit() incorrect" );

		k = TBigInteger( 0x80010000 ) << 11;
		log() << "k = " << k << endl;
		if( k.getBlock(1) != 0x400 || k.getBlock(0) != 0x08000000 )
			throw logic_error( "Shift left incorrect (non block multiple)" );
		log() << "k.highestBit() = " << k.highestBit() << "; k.lowestBit() = " << k.lowestBit() << endl;
		if( k.lowestBit() != 27 )
			throw logic_error( "TBigInteger::lowestBit() incorrect" );
		if( k.highestBit() != 42 )
			throw logic_error( "TBigInteger::highestBit() incorrect" );

		k = TBigInteger( 0x80010000 ) << 128;
		// Hex:       Binary:
		//            159  152      144      136      128
		// 80010000   10000000 00000001 00000000 00000000
		//                 120      112      104       96
		// 00000000   00000000 00000000 00000000 00000000
		//                  88       80       72       64
		// 00000000   00000000 00000000 00000000 00000000
		//                  56       48       40       32
		// 00000000   00000000 00000000 00000000 00000000
		//                  24       16        8        0
		// 00000000   00000000 00000000 00000000 00000000
		//
		// lowestBit = 144
		// highestBit = 159
		//
		log() << "k = " << k << endl;
		if( k.getBlock(4) != 0x80010000 || k.getBlock(3) != 0x0
				|| k.getBlock(3) != 0x0 || k.getBlock(2) != 0x0
				|| k.getBlock(1) != 0x0 || k.getBlock(0) != 0x0 )
			throw logic_error( "Shift left incorrect" );
		log() << "k.highestBit() = " << k.highestBit() << "; k.lowestBit() = " << k.lowestBit() << endl;
		if( k.lowestBit() != 144 )
			throw logic_error( "TBigInteger::lowestBit() incorrect" );
		if( k.highestBit() != 159 )
			throw logic_error( "TBigInteger::highestBit() incorrect" );

		k = sixtyFourB & sixtyFourC;
		log() << "64bit: " << sixtyFourB << " AND " << sixtyFourC << " = " << k << endl;
		// 0xffffffffdddddddd AND 0xeeeeeeeecccccccc =
		k = sixtyFourB | sixtyFourC;
		log() << "64bit: " << sixtyFourB << " OR  " << sixtyFourC << " = " << k << endl;
		// 0xffffffffdddddddd OR 0xeeeeeeeecccccccc =
		k = sixtyFourB ^ sixtyFourC;
		log() << "64bit: " << sixtyFourB << " XOR " << sixtyFourC << " = " << k << endl;
		// 0xffffffffdddddddd XOR 0xeeeeeeeecccccccc =
		k = ~sixtyFourB;
		log() << "64bit: NOT " << sixtyFourB << " = " << k << endl;
		// 0xffffffffdddddddd NOT 0xeeeeeeeecccccccc =


		log(TLog::Status) << "Testing arithmetic operators on large numbers" << endl;

		k = sixtyFourB + sixtyFourC;
		log() << "64bit: " << sixtyFourB << " + " << sixtyFourC << " = " << k << endl;
		if( k != TBigInteger( 0x1, 0xEEEEEEEE, 0xAAAAAAA9 ) )
			throw logic_error( "64bit addition incorrect" );

		k = sixtyFourB - sixtyFourC;
		log() << "64bit: " << sixtyFourB << " - " << sixtyFourC << " = " << k << endl;
		if( k != TBigInteger( 0x11111111, 0x11111111 ) )
			throw logic_error( "64bit subtraction incorrect" );

		k = thirtyTwoB * thirtyTwoC;
		log() << "32bit: " << thirtyTwoB << " * " << thirtyTwoC << " = " << k << endl;
		// 0xffffdddd * 0xeeeecccc = 0xeeeeacef9e26e81c

		k = sixtyFourB * sixtyFourC;
		log() << "64bit: " << sixtyFourB << " * " << sixtyFourC << " = " << k << endl;
		// 0xffffffffdddddddd * 0xeeeeeeeecccccccc = 0xeeeeeeeeacf1357826af37c081b4e81c

		k = thirtyTwoB / thirtyTwoC;
		j = thirtyTwoB % thirtyTwoC;
		log() << "32bit: " << thirtyTwoB << " / " << thirtyTwoC << " = " << k << " r " << j << endl;
		// 0xffffdddd / 0xeeeecccc = 0x1 r 0x11111111
		if( k != 0x1 || j != 0x11111111 )
			throw logic_error( "32bit divide incorrect" );

		k = sixtyFourB / sixtyFourC;
		j = sixtyFourB % sixtyFourC;
		log() << "64bit: " << sixtyFourB << " / " << sixtyFourC << " = " << k << " r " << j << endl;
		// 0xffffffffdddddddd / 0xeeeeeeeecccccccc = 0x1 r 0x1111111111111111
		if( k != 0x1 || j != TBigInteger( 0x11111111, 0x11111111 ) )
			throw logic_error( "64bit divide incorrect" );

		thirtyTwoB = TBigInteger( 0x12345678 );
		thirtyTwoC = TBigInteger( 0xaaaa );
		k = thirtyTwoB / thirtyTwoC;
		j = thirtyTwoB % thirtyTwoC;
		log() << "32bit: " << thirtyTwoB << " / " << thirtyTwoC << " = " << k << " r " << j << endl;
		if( k != 0x1b4e || j != 0x68ac )
			throw logic_error( "32bit divide incorrect" );

		log(TLog::Status) << "Testing that (x+1)-x is always 1" << endl;
		// Test overflow handling
		j = 1;
		for( unsigned int b = 0; b < 4*1000; b++ ) {
			k = j + 1;
			if( (k-j) != 1 ) {
				log(TLog::Error) << "[" << b << "] j = " << j
					<< "; j+1 = " << k
					<< "; diff = " << (k-j) << endl;
				throw logic_error("Adding one didn't make numbers one apart");
			}

//			log() << "j = " << j << "; k = " << k << endl;

			// Faults will occur around binary divisions, so we'll pass
			// through all of them by shifting left and pushing in
			// another 1.  This gets us lots of 0xff..ff blocks.
			j <<= 1;
			j += 1;
		}
		log() << "Finished overflow test" << endl;
		log() << "k = " << k << "; k.highestBit() = " << k.highestBit()
			<< "; k.lowestBit() = " << k.lowestBit() << endl;
		if( k.highestBit() != k.lowestBit() || k.highestBit() != 4*1000 )
			throw logic_error( "0xff..ff plus 1 should always contain only one bit" );

		log(TLog::Status) << "Undoing left shifts" << endl;
		for( unsigned int b = 0; b < 4*1000; b++ ) {
			j >>= 1;
		}
		if( j != 1 )
			throw logic_error( "Right shift didn't undo left shift" );


	} catch( exception &e ) {
		log(TLog::Error) << e.what() << endl;
		return 255;
	}


	try {
		log(TLog::Status) << "Speed test" << endl;
		TTimeAbsolute start, stop;

		// ---

		unsigned char z;

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			z = z * 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned char> multiplications = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		unsigned int x;

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			x = x * 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned int> multiplications = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		unsigned long long w;

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			w = w * 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned long long> multiplications = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		TBigInteger five(5);
		TBigInteger y;

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			y = five * five;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <TBigInteger> multiplications = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			z = z / 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned char> divisions = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			x = x / 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned int> divisions = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			w = w / 5;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <unsigned long long> divisions = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

		// ---

		start.setNow();
		for( unsigned int b = 0; b < LOOPS; b++ ) {
			y = y / five;
		}
		stop.setNow();
		log() << "Time for " << LOOPS << " <TBigInteger> divisions = "
			<< (stop - start) << " (" << (stop-start).perSecond(LOOPS) << ")" << endl;

	} catch( exception &e ) {
		log(TLog::Error) << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif


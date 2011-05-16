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
#include <stdint.h>
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
// Function:	TGenericBigInteger :: fromString
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &TGenericBigInteger<tLittleInteger>::fromString( const string &s, unsigned int Base )
{
	string::size_type pos;
	unsigned int ch;

	// Start at zero
	(*this) = 0;

	pos = 0;
	while( pos < s.size() ) {
		ch = static_cast<unsigned char>( s[pos] );

//		cerr << "'" << (char)(ch) << "' ";

		ch = fromCharacter( ch, Base );
		if( ch > 0xff )
			break;

//		cerr << "shl, adding " << ch
//			<< " to " << hex << *this << dec << "*" << Base;

		// Shift left by base, and add the next digit
		(*this) *= Base;
		(*this) += TGenericBigInteger( ch );

//		cerr << " = " << hex << *this << dec << endl;;
//		cerr << " = " << toString(58) << endl;;

		pos++;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigInteger :: fromCharacter
// Description:
//
template <typename tLittleInteger>
unsigned int TGenericBigInteger<tLittleInteger>::fromCharacter( unsigned int ch, unsigned int Base ) const
{
	// 80 byte lookup table for reversing the above map.  It is the
	// indexed by (ASCII_VALUE - '+') because '+' is the lowest ASCII
	// value in the above map, and we rely on the user of the table to
	// ensure they don't look up negative values.  Invalid lookups are
	// marked by 0xff, which is outside the 0-63 base64 range, so would
	// never occur naturally.
	static const uint8_t ASCIIToBase64[] = {
		// Output (0-63)                                 // Input (ASCII)
		0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,  // + . . . / 0 1 2
		0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,  // 3 4 5 6 7 8 9 .
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,  // . . . . . . A B
		0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,  // C D E F G H I J
		0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,  // K L M N O P Q R
		0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,  // S T U V W X Y Z
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,  // . . . . . . a b
		0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,  // c d e f g h i j
		0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,  // k l m n o p q r
		0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33   // s t u v w x y z
	};
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base <= 10 ) {
		if( isdigit(ch) ) {
			ch = (ch - '0');
		} else {
			ch = INVALID;
		}
	} else if( Base <= 10 + 26 ) {
		if( ch <= '9' ) {
			ch = ch - '0';
		} else if( ch <= 'Z' ) {
			ch = ch - 'A' + 0xa;
		} else if( ch <= 'z' ) {
			ch = ch - 'a' + 0xa;
		} else {
			ch = INVALID;
		}
	} else if( Base <= 10 + 26*2 ) {
		if( ch <= '9' ) {
			ch = ch - '0';
		} else if( ch <= 'Z' ) {
			ch = ch - 'A' + 0xa;
		} else if( ch <= 'z' ) {
			ch = ch - 'a' + 10 + 26;
		} else {
			ch = INVALID;
		}
	} else if( Base <= 64 ) {
		if( ch >= '+' && ch-'+' <= sizeof(ASCIIToBase64) ) {
			ch = ASCIIToBase64[ch-'+'];
			if( ch == 0xff )
				ch = INVALID;
		} else {
			ch = INVALID;
		}
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TGenericBigInteger :: toString
// Description:
//
template <typename tLittleInteger>
string TGenericBigInteger<tLittleInteger>::toString( unsigned int Base ) const
{
	unsigned int ch;
	string output;

	TGenericBigInteger quotient, remainder;

	// Start with the whole number
	quotient = *this;

	// e.g.
	// 1234567890123 / 10  = 123456789012 r 3
	//  123456789012 / 10  = 12345678901 r 2
	//   12345678901 / 10  = 1234567890 r 1
	//    1234567890 / 10  = 123456789 r 0
	//     123456789 / 10  = 12345678 r 9
	//      12345678 / 10  = 1234567 r 8
	//       1234567 / 10  = 123456 r 7
	//        123456 / 10  = 12345 r 6
	//         12345 / 10  = 1234 r 5
	//          1234 / 10  = 123 r 4
	//           123 / 10  = 12 r 3
	//            12 / 10  = 1 r 2
	//             1 / 10  = 0 r 1
	// Read the remainder from bottom to top.  Each remainder is the
	// "nth" symbol in base-N display.  We just look up what character
	// should be displayed for that number and we have our conversion.

//	cerr << hex << quotient << dec << endl;
	do {
		remainder = quotient;
		remainder.divideWithRemainder( Base, quotient );

//		cerr << hex << quotient << dec << " r " << remainder.getBlock(0) << endl;

		// Remainder is guaranteed to be less than Base, and Base is an
		// unsigned int, so we're safe using the zero block untouched
		ch = toCharacter( remainder.getBlock(0), Base );

		// Hitting an invalid conversion is a serious error
		if( ch > 0xff )
			logic_error("toCharacter() returned INVALID -- impossible");

		// Prepend character
		output = string() + static_cast<char>(ch) + output;
	} while( !quotient.isZero() );

	return stringPad( output, Base );
//	return output;
}

//
// Function:	TGenericBigInteger :: toCharacter
// Description:
//
template <typename tLittleInteger>
unsigned int TGenericBigInteger<tLittleInteger>::toCharacter( unsigned int ch, unsigned int Base ) const
{
	static const char Base64ToASCII[65] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base <= 10 ) {
		ch = '0' + ch;
	} else if( Base <= 10 + 26 ) {
		if( ch <= 9 ) {
			ch = ch + '0';
		} else if( ch <= 9 + ('z'-'a') ) {
			ch = (ch - 0xa) + 'a';
		} else {
			ch = INVALID;
		}
	} else if( Base <= 10 + 26*2 ) {
		if( ch <= 9 ) {
			ch = ch + '0';
		} else if( ch <= 9 + ('Z'-'A') ) {
			ch = (ch - 0xa) + 'A';
		} else if( ch <= 9 + ('Z'-'A') + ('z'-'a') ) {
			ch = (ch-('Z'-'A')-0xa) + 'a';
		} else {
			ch = INVALID;
		}
	} else if( Base <= 64 ) {
		if( ch <= sizeof(Base64ToASCII) )
			ch = Base64ToASCII[ch];
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TGenericBigInteger :: stringPad
// Description:
//
template <typename tLittleInteger>
string TGenericBigInteger<tLittleInteger>::stringPad( const string &s, unsigned int Base ) const
{
	static const char BASE64PAD = '=';
	string output;

	if( Base == 64 ) {
		output = s;
		// Pad to ensure we supply a multiple of four bytes
		while( (output.length() & 0x03) != 0 )
			output.append(1, BASE64PAD );
	} else {
		output = s;
	}

	return output;
}

//
// Function:	TGenericBigInteger :: toBytes
// Description:
//
template <typename tLittleInteger>
string TGenericBigInteger<tLittleInteger>::toBytes( unsigned int Minimum ) const
{
	typename tLittleDigitsVector::const_reverse_iterator it;
	string output;

	// Byte streams are big-endian, so we traverse backwards
	it = LittleDigits.rbegin();

	// Warn string how much we're going to put in it
	output.reserve( sizeof(tLittleInteger) * LittleDigits.size() );

	while( it != LittleDigits.rend() ) {
		// Getting hard to stay generic here; the compiler should
		// optimise this as appropriate.
		switch(sizeof(tLittleInteger)) {
			// Big endian, so MSBs go first
			case 4:
				output += ((*it) & 0xff000000) >> 24;
				output += ((*it) & 0xff0000) >> 16;
			case 2:
				output += ((*it) & 0xff00) >> 8;
			case 1:
				output += ((*it) & 0xff) >> 0;
		}

		it++;
	}

	string::size_type pos;

	pos = output.find_first_not_of( '\0' );

	if( pos != string::npos && output.size() > Minimum ) {
		// If we would clip so many zeroes that we would shrink the
		// string below the minimum size...
		if( output.size() - pos < Minimum )
			// ... then only reduce to the minimum size
			pos = output.size() - Minimum;

		// Strip leading zeroes
		output.erase( 0, pos );
	}
	if( output.size() < Minimum )
		output = string().assign( Minimum - output.size(), '\0' ) + output;

	// Equal sizes, equal content
	return output;
}

//
// Function:	TGenericBigInteger :: fromBytes
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::fromBytes( const string &s )
{
	tLittleInteger Accumulator;
	int pos = s.size();
	LittleDigits.clear();

	// We start at the right hand end of the string, because strings are
	// stored big endian and our LittleDigits array is little endian

	//           MSB---------------------LSB
	// string    xx xxxx xxxx xxxx xxxx xxxx
	// pos    -2    2    6    10   14   18
	// offset  0123 0123 0123 0123 0123 0123
	// Digits   [5]  [4]  [3]  [2]  [1]  [0]

	while( pos > 0 ) {
		Accumulator = 0;

		// Move backwards one accumulator's worth
		pos -= sizeof( tLittleInteger );

		switch( sizeof( tLittleInteger ) ) {
			case 4:
				if( pos + 3 >= 0 )
					Accumulator |= static_cast<unsigned char>(s[pos+3]) << 0;
				if( pos + 2 >= 0 )
					Accumulator |= static_cast<unsigned char>(s[pos+2]) << 8;
			case 2:
				if( pos + 1 >= 0 )
					Accumulator |= static_cast<unsigned char>(s[pos+1]) << 16;
			case 1:
				if( pos + 0 >= 0 )
					Accumulator |= static_cast<unsigned char>(s[pos+0]) << 24;
		}

		LittleDigits.push_back( Accumulator );
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

	for( tIndex i = 0; i < B.LittleDigits.size(); i++ ) {
		for( tIndex j = 0; j < C.LittleDigits.size(); j++ ) {
			blockMultiply( R1, R0, B.LittleDigits[i], C.LittleDigits[j] );

			// Use TGenericBigInteger to do the digit shift and addition with
			// carry
			(*this) += TGenericBigInteger<tLittleInteger>(R1,R0).blockShiftLeft( i + j );;
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
TGenericBigInteger<tLittleInteger> &TGenericBigInteger<tLittleInteger>::divideWithRemainder(const TGenericBigInteger<tLittleInteger> &d, TGenericBigInteger<tLittleInteger> &Q)
{
	// The mathematical operators will fail if they operate directly on
	// themselves.  This macro detects a self operation and reruns the
	// operation using a temporary copy.
	if( this == &d || this == &Q ) {
		TGenericBigInteger<tLittleInteger> Temporary;
		Temporary.divideWithRemainder(d,Q);
		*this = Temporary;
		return *this;
	}

	// "this" is the numerator; D is the denominator, and we'll be
	// storing the remainder in this and the quotient in Q.

	TGenericBigInteger<tLittleInteger> D(d);

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

	Q = 0;

	if( denominatorHighBit > numeratorHighBit ) {
		// The numerator is the remainder and the quotient is zero
		// because the denominator is larger than the numerator
		return *this;
	}

//	cerr << "NHB - DHB      = " << numeratorHighBit << " - " << denominatorHighBit << endl;
//	cerr << "Numerator      = " << hex << *this << endl;
//	cerr << "Denominator    = " << D << dec << endl;

	// First we shift the denominator up to have its high bit in the
	// same place as the numerator high bit
	D <<= numeratorHighBit - denominatorHighBit;

	tIndex shifts = numeratorHighBit - denominatorHighBit + 1;

//	cerr << "Denominator[0] = " << hex << D << dec << endl;
//	cerr << "shifts         = " << shifts << endl;

	while( shifts > 0 ) {
		shifts--;
		Q <<= 1;

		// If the current denominator is less than or equal to the
		// numerator, then it will divide...
		if( D <= *this ) {
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

//	cerr << hex;
//	cerr << "Remainder      = " << *this << endl;
//	cerr << "Denominator    = " << D << endl;
//	cerr << "Quotient       = " << Q << endl;
//	cerr << dec;

	return *this;
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

	R.LittleDigits.clear();

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

	return R;
}

//
// Function:	TGenericBigInteger :: reversedBytes
// Description:
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger>
TGenericBigInteger<tLittleInteger>::reversedBytes() const
{
	TGenericBigInteger<tLittleInteger> R;
	typename tLittleDigitsVector::const_reverse_iterator it;

	R.LittleDigits.clear();

	// Start at most significant end of source
	it = LittleDigits.rbegin();

	while( it != LittleDigits.rend() ) {
		tLittleInteger Hold = (*it);
		switch( sizeof(Hold) ) {
			case 4:
				// x0 x1 x2 x3 -> x3 x2 x1 x0
				Hold = (Hold & 0xff000000) >> 24
					| (Hold & 0x00ff0000) >> 8
					| (Hold & 0x0000ff00) << 8
					| (Hold & 0x000000ff) << 24;
				break;
			case 2:
				// x0 x1 -> x1 x0
				Hold = (Hold & 0xff00) >> 8
					| (Hold & 0x00ff) << 8;
		}
		// Write out to the least significant end first
		R.LittleDigits.push_back( Hold );
		it++;
	}

	R.normalise();

	return R;
}

//
// Function:	TGenericBigInteger :: operator<<=
// Description:
/// Implement A = A << B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator<<=( tIndex b )
{
	typename tLittleDigitsVector::iterator it;
	tLittleInteger overflowBits, lastBits = 0;

	// The block part can be done with blockShiftLeft()
	blockShiftLeft( b / bitsPerBlock );

	// The left over must be done with a bit shift
	b = b % bitsPerBlock;

	if( b == 0 )
		return *this;

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

	return *this;
}

//
// Function:	TGenericBigInteger :: operator>>=
// Description:
/// Implement A = A >> B
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::operator>>=( tIndex b )
{
	typename tLittleDigitsVector::reverse_iterator it;
	tLittleInteger underflowBits, lastBits = 0;

	// The block part can be done with blockShiftRight()
	blockShiftRight( b / bitsPerBlock );
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

	return *this;
}

//
// Function:	TGenericBigInteger :: blockShiftLeft
// Description:
// xxx yyy zzz -> xxx yyy zzz 0
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::blockShiftLeft( tIndex b )
{
	typename tLittleDigitsVector::const_iterator itA;

	// Leave zero and invalid as they are
	if( isZero() || !isValid() || b == 0 )
		return *this;

	tLittleDigitsVector DigitsCopy(LittleDigits);
	itA = DigitsCopy.begin();

	LittleDigits.clear();
	LittleDigits.reserve( DigitsCopy.size() + b );

	// b new blocks are a copy of the last b blocks
	while( b-- ) {
		LittleDigits.push_back( 0 );
	}

	while( itA != DigitsCopy.end() ) {
		LittleDigits.push_back(*itA);
		itA++;
	}

	return *this;
}

//
// Function:	TGenericBigInteger :: blockShiftRight
// Description:
// xxx yyy zzz -> xxx yyy
//
template <typename tLittleInteger>
TGenericBigInteger<tLittleInteger> &
TGenericBigInteger<tLittleInteger>::blockShiftRight( tIndex b )
{
	typename tLittleDigitsVector::iterator itA;
	typename tLittleDigitsVector::const_iterator itB;

	if( !isValid() || isZero() || b == 0 )
		return *this;

	// Start at least significant end of source
	itA = LittleDigits.begin();
	itB = LittleDigits.begin();

	itB += b;
	if( itB == LittleDigits.end() ) {
		LittleDigits.clear();
		LittleDigits.push_back(0);
		return *this;
	}

	while( itB != LittleDigits.end() ) {
		cerr << *this << endl;
		*itA = *itB;
		itA++;
		itB++;
	}

	// Remove the last element
	LittleDigits.pop_back();

	return *this;
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

	if( s.flags() & ostream::hex ) {
		// We don't use toString() for hex conversion as we can do it
		// faster directly, but also: how would we put debug output in
		// toString() if it used itself to display that output?
		while( it1 != LittleDigits.rend() ) {
			// There are four bits per hex digit, so ensure that every block
			// (except the first) shows the appropriate zero padding.
			if( it1 != LittleDigits.rbegin() )
				s << setw( bitsPerBlock/4 ) << setfill('0');
			s << (*it1);
			it1++;
		}
	} else if( s.flags() & ostream::oct ) {
		s << toString(8);
	} else if( s.flags() & ostream::dec ) {
		s << toString(10);
	} else {
	}

	return s;
}

// ----------

//
// Function:	TGenericBigSignedInteger :: TGenericBigSignedInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::TGenericBigSignedInteger( tLittleInteger r1, tLittleInteger r0 ) :
	TGenericBigInteger<tLittleInteger>(r1,r0),
	Negative(false)
{
}

//
// Function:	TGenericBigSignedInteger :: TGenericBigSignedInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::TGenericBigSignedInteger( tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 ) :
	TGenericBigInteger<tLittleInteger>(r2,r1,r0),
	Negative(false)
{
}

//
// Function:	TGenericBigSignedInteger :: TGenericBigSignedInteger
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::TGenericBigSignedInteger( tLittleInteger r3, tLittleInteger r2, tLittleInteger r1, tLittleInteger r0 ) :
	TGenericBigInteger<tLittleInteger>(r3,r2,r1,r0),
	Negative(false)
{
}

//
// Function:	TGenericBigSignedInteger :: fromString
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &TGenericBigSignedInteger<tLittleInteger>::fromString( const string &s, unsigned int Base )
{
	string::size_type pos;

	pos = s.find('-', 0);
	if( pos != string::npos ) {
		TGenericBigInteger<tLittleInteger>::fromString(
				s.substr(pos,s.size()-pos), Base);
		Negative = true;
	} else {
		TGenericBigInteger<tLittleInteger>::fromString(s, Base);
		Negative = false;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigSignedInteger :: toString
// Description:
//
template <typename tLittleInteger>
string TGenericBigSignedInteger<tLittleInteger>::toString( unsigned int Base ) const
{
	if( Negative ) {
		return string("-") + TGenericBigInteger<tLittleInteger>::toString( Base );
	} else {
		return TGenericBigInteger<tLittleInteger>::toString( Base );
	}
}

//
// Function:	TGenericBigSignedInteger :: operator=
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &TGenericBigSignedInteger<tLittleInteger>::operator=( long long t )
{
	if( t < 0 ) {
		TGenericBigInteger<tLittleInteger>::operator=(-t);
		Negative = true;
	} else {
		TGenericBigInteger<tLittleInteger>::operator=(t);
		Negative = false;
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigSignedInteger :: fromBytes
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::fromBytes( const string &s )
{
	// fromBytes() can only create positive numbers
	Negative = false;
	TGenericBigInteger<tLittleInteger>::fromBytes(s);
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: compareTo
// Description:
// The sign of each of the values can flip the result, so we do the
// unsigned comparison and then adjust for sign.
//
template <typename tLittleInteger>
typename TGenericBigSignedInteger<tLittleInteger>::eComparisonResult
TGenericBigSignedInteger<tLittleInteger>::compareTo( const TGenericBigSignedInteger<tLittleInteger> &C ) const
{
	switch( TGenericBigInteger<tLittleInteger>::compareTo(C) ) {
		case TGenericBigInteger<tLittleInteger>::LessThan:
			if( C.isNegative() ) {
				return TGenericBigInteger<tLittleInteger>::GreaterThan;
			} else {
				return TGenericBigInteger<tLittleInteger>::LessThan;
			}
			break;
		case TGenericBigInteger<tLittleInteger>::EqualTo:
			if( isNegative() != C.isNegative() ) {
				if( isNegative() ) {
					return TGenericBigInteger<tLittleInteger>::LessThan;
				} else {
					return TGenericBigInteger<tLittleInteger>::GreaterThan;
				}
			} else {
				return TGenericBigInteger<tLittleInteger>::EqualTo;
			}
			break;
		case TGenericBigInteger<tLittleInteger>::GreaterThan:
			if( isNegative() ) {
				return TGenericBigInteger<tLittleInteger>::LessThan;
			} else {
				return TGenericBigInteger<tLittleInteger>::GreaterThan;
			}
			break;
	}
	throw logic_error("How did we get here (TGenericBigSignedInteger<tLittleInteger>()");
}

//
// Function:	TGenericBigSignedInteger :: normalise
// Description:
/// Remove sign on zeroes.
//
template <typename tLittleInteger>
void TGenericBigSignedInteger<tLittleInteger>::normalise()
{
	// We'll define zero as positive
	if( isZero() )
		Negative = false;
	TGenericBigInteger<tLittleInteger>::normalise();
}

//
// Function:	TGenericBigSignedInteger :: operator+=
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator+=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	if( isNegative() == B.isNegative() ) {
		// If both are negative or both are positive then simple
		// addition will do, and our sign can stay as it is
		TGenericBigInteger<tLittleInteger>::operator+=(B);
	} else {
		// We're negative and we want to add a positive number, that is
		// the same as reducing us by that positive number, but we must
		// be careful not to ask the base class to provide a negative
		// number, which we do by comparing absolutes to find the right
		// way round to do the subtraction.
		if( TGenericBigInteger<tLittleInteger>::compareTo(B) == TGenericBigInteger<tLittleInteger>::LessThan ) {
			// Our absolute is smaller than the subtractand absolute.
			//   e.g.  -5 + 10 = 5
			// We'll have to do the subtraction we can do and invert our
			// sign
			TGenericBigSignedInteger<tLittleInteger> X( *this );
			*this = B;
			TGenericBigInteger<tLittleInteger>::operator-=(X);
			setNegative( !X.isNegative() );
		} else {
			// Our absolute is larger than the subtractand absolute.
			//   e.g.  -10 + 5 = -5
			// Just do it and leave our sign as it is
			TGenericBigInteger<tLittleInteger>::operator-=(B);
		}
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator-=
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator-=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	if( isNegative() != B.isNegative() ) {
		// Negative minus positive is addition with sign left as it is
		// Positive minus negative is addition with sign left as it is
		TGenericBigInteger<tLittleInteger>::operator+=(B);
	} else {
		// Find out which is the bigger
		if( TGenericBigInteger<tLittleInteger>::compareTo(B) == TGenericBigInteger<tLittleInteger>::LessThan ) {
			// Our absolute is less than the subtractand absolute
			//  e.g.  -5 - -10 = 5
			// Do the subtraction we can do and negate
			TGenericBigInteger<tLittleInteger> X( *this );
			*this = B;
			TGenericBigInteger<tLittleInteger>::operator-=(X);
			negate();
		} else {
			// Our absolute is larger than the subtractand absolute
			//  e.g. -10 - -5 = -5
			TGenericBigInteger<tLittleInteger>::operator-=(B);
			// Sign correct
		}
	}

	normalise();

	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator*=
// Description:
/// Implement A = A * B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator*=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	// Base class multiply will be fine, and we'll special case the sign
	TGenericBigInteger<tLittleInteger>::operator*=(B);
	Negative = (isNegative() != B.isNegative());
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: divideWithRemainder
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &TGenericBigSignedInteger<tLittleInteger>::divideWithRemainder(const TGenericBigSignedInteger<tLittleInteger> &d, TGenericBigSignedInteger<tLittleInteger> &Q)
{
	// The sign of the quotient is determined by the sign of the
	// numerator and denominator
	Q.setNegative( isNegative() != d.isNegative() );
	// C++ doesn't define the sign of the remainder, C 1999 defines the
	// remainder as having the same sign as the dividend; I prefer a
	// standardised result; and when C++ does standardise it's more
	// likely to be the same as C than not.  *this is the dividend so we
	// simply leave the sign as it is
//	Negative = isNegative();
	// Base class divide will be fine, and we'll special case the sign
	TGenericBigInteger<tLittleInteger>::divideWithRemainder(d, Q);
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator&=
// Description:
/// Implement A = A & B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator&=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	TGenericBigInteger<tLittleInteger>::operator&=(B);
	// Bitwise AND, so we'll treat the negative flag as just another bit
	Negative = (isNegative() && B.isNegative());
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator|=
// Description:
/// Implement A = A | B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator|=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	TGenericBigInteger<tLittleInteger>::operator|=(B);
	// Bitwise, so we'll treat the negative flag as just another bit
	Negative = (isNegative() || B.isNegative());
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator^=
// Description:
/// Implement A = A ^ B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator^=(
		const TGenericBigSignedInteger<tLittleInteger> &B)
{
	TGenericBigInteger<tLittleInteger>::operator^=(B);
	// Bitwise, so we'll treat the negative flag as just another bit
	Negative = (isNegative() != B.isNegative());
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator~
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::operator~() const
{
	TGenericBigSignedInteger<tLittleInteger> X( TGenericBigInteger<tLittleInteger>::operator~() );
	// Bitwise, so we'll treat the negative flag as just another bit
	X.setNegative( !isNegative() );
	return X;
}

//
// Function:	TGenericBigSignedInteger :: operator-
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::operator-() const
{
	return TGenericBigSignedInteger(*this).negate();
}

//
// Function:	TGenericBigSignedInteger :: reversedBytes
// Description:
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>
TGenericBigSignedInteger<tLittleInteger>::reversedBytes() const
{
	TGenericBigSignedInteger<tLittleInteger> X( TGenericBigInteger<tLittleInteger>::reversedBytes() );
	return X;
}

//
// Function:	TGenericBigSignedInteger :: operator<<=
// Description:
/// Implement A = A << B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator<<=( tIndex b )
{
	TGenericBigInteger<tLittleInteger>::operator<<=(b);
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: operator>>=
// Description:
/// Implement A = A >> B
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::operator>>=( tIndex b )
{
	TGenericBigInteger<tLittleInteger>::operator>>=(b);
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: blockShiftLeft
// Description:
// xxx yyy zzz -> xxx yyy zzz 0
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::blockShiftLeft( tIndex b )
{
	TGenericBigInteger<tLittleInteger>::blockShiftLeft(b);
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: blockShiftRight
// Description:
// xxx yyy zzz -> xxx yyy
//
template <typename tLittleInteger>
TGenericBigSignedInteger<tLittleInteger> &
TGenericBigSignedInteger<tLittleInteger>::blockShiftRight( tIndex b )
{
	TGenericBigInteger<tLittleInteger>::blockShiftRight(b);
	normalise();
	return *this;
}

//
// Function:	TGenericBigSignedInteger :: printOn
// Description:
//
template <typename tLittleInteger>
ostream &TGenericBigSignedInteger<tLittleInteger>::printOn( ostream &s ) const
{
	if( s.flags() & ostream::hex ) {
		if( Negative )
			s << "-";
		TGenericBigInteger<tLittleInteger>::printOn(s);
	} else if( s.flags() & ostream::oct ) {
		s << toString(8);
	} else if( s.flags() & ostream::dec ) {
		s << toString(10);
	} else {
	}

	return s;
}


// -------------- Explicit template instantiations
template class TGenericBigInteger<unsigned int>;
template class TGenericBigSignedInteger<unsigned int>;


// -------------- Function definitions


#ifdef UNITTEST
#include "logstream.h"
#include "extratime.h"
#include <sstream>

// -------------- main()

int main( int argc, char *argv[] )
{
	static const unsigned int LOOPS = 1000000;

	try {
		log(TLog::Status) << "Testing constructors and initialisation" << endl;

		TBigInteger i("99999999999999999999"); // 0x56BC75E2D630FFFFF
		TBigInteger j("56bC75E2D630ffFFF",16); // 0x56BC75E2D630FFFFF
		TBigInteger k("56bC75E2D630ffFFF",36);
		TBigInteger l("56bC75E2D630ffFFF",56);
		TBigInteger m;

		log() << "i = " << hex << i << dec << endl;
		if( i.getBlock(2) != 0x5 || i.getBlock(1) != 0x6bc75e2d || i.getBlock(0) != 0x630fffff )
			throw logic_error("Assignment from decimal-representing string incorrect");
		log() << "j = " << hex << j << dec << endl;
		if( j != i )
			throw logic_error("Assignment from hexadecimal-representing string incorrect");
		if( j.toBytes() != string("\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",9) ) {
			log() << "bytes = ";
			TLog::hexify(log(), j.toBytes(10) );
			log() << endl;
			throw logic_error("Comparison in byte mode failed");
		}
		if( j.toBytes(10) != string("\x00\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",10)
				|| j.toBytes(11) != string("\x00\x00\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",11)
				|| j.toBytes(12) != string("\x00\x00\x00\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",12)
				|| j.toBytes(13) != string("\x00\x00\x00\x00\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",13) ) {
			throw logic_error("Minimum over string size failed in conversion to bytes");
		}
		if( j.toBytes(5) != string("\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",9) ) {
			throw logic_error("Minimum under string size failed in conversion to bytes");
		}

		log() << "k = " << hex << k << dec << endl;
		log() << "l = " << hex << l << dec << endl;

		m.fromBytes( string("\x05\x6b\xC7\x5E\x2D\x63\x0f\xfF\xFF",9) );
		if( m != j ) {
			log() << "m = " << hex << m << dec << endl;
			throw logic_error("Assignment from bytes failed");
		}

	} catch( exception &e ) {
		log(TLog::Error) << e.what() << endl;
		return 255;
	}


	try {
		//      2^32 = 4294967296
		//      2^64 = 18446744073709551616
		TBigInteger i("99999999999999999999"); // 0x56BC75E2D630FFFFF
		TBigInteger thirtyTwoB( 0xffffdddd ), thirtyTwoC( 0xeeeecccc );
		TBigInteger sixtyFourB( 0xffffffff, 0xdddddddd ),
					sixtyFourC( 0xeeeeeeee, 0xcccccccc );
		TBigInteger j, k;

		j = 1LL;

		log(TLog::Status) << "Testing bit operators on large numbers" << endl;

		k = TBigInteger( 0x1, 0xffffffff ) << 1;
		log() << "k = " << hex << k << dec << endl;
		if( k.getBlock(1) != 0x3 || k.getBlock(0) != 0xfffffffe )
			throw logic_error( "Shift left incorrect (non block multiple, edge)" );
		log() << "k.highestBit() = " << k.highestBit() << "; k.lowestBit() = " << k.lowestBit() << endl;
		if( k.lowestBit() != 1 )
			throw logic_error( "TBigInteger::lowestBit() incorrect" );
		if( k.highestBit() != 33 )
			throw logic_error( "TBigInteger::highestBit() incorrect" );

		k = TBigInteger( 0x80010000 ) << 11;
		log() << "k = " << hex << k << dec << endl;
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
		log() << "k = " << hex << k << dec << endl;
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
		log() << hex;
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
		log() << dec;
		// reverse
		k = TBigInteger("0102030405060708090a0b0c0d0e0f10", 16);
		log() << "64bit: reverse 0x" << hex << k << " = 0x" << k.reversedBytes() << dec << endl;
		if( k.reversedBytes() != TBigInteger("100f0e0d0c0b0a090807060504030201",16) )
			throw logic_error( "reversedBytes() failed" );

		log(TLog::Status) << "Testing arithmetic operators on large numbers" << endl;
		log() << hex;

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

		// Divide by a little number
		k = sixtyFourB / 10;
		j = sixtyFourB % 10;
		log() << "64bit: " << sixtyFourB << " / 10 = " << k << " r " << j << endl;
		// 0xffffffffdddddddd / 10 = 0x19999999962fc962 r 9
		if( j != 9 || k != TBigInteger( 0x19999999, 0x962fc962) )
			throw logic_error( "64bit divide incorrect" );

		// Divide by a little number
		TBigInteger biggest( 0x1000000, 0x00000000, 0x00000000, 0x00000000 );
		k = biggest / 64;
		j = biggest % 64;
		log() << "64bit: " << biggest << " / 64 = " << k << " r " << j << endl;
		// 0xffffffffdddddddd / 10 = 0x19999999962fc962 r 9
		if( j != 0 || k != TBigInteger(0x40000,0x00000000,0x00000000,0x00000000) )
			throw logic_error( "64bit divide incorrect" );

		log() << dec;

		log(TLog::Status) << "Testing that (x+1)-x is always 1" << endl;
		// Test overflow handling
		j = 1;
		for( unsigned int b = 0; b < 4*1000; b++ ) {
			k = j + 1;
			if( (k-j) != 1 ) {
				log(TLog::Error) << "[" << b << "] j = " << j
					<< "; j+1 = " << hex << k << dec
					<< "; diff = " << hex << (k-j) << dec << endl;
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
		log() << "k = " << hex << k << dec << "; k.highestBit() = " << k.highestBit()
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
		log() << "Testing signed arithmetic" << endl;

		TBigInteger neg1(-1), pos1(-neg1);
		TBigInteger pos10(10), neg10(-pos10);

		log() << "neg1  = " << neg1 << " (negated: " << -neg1 << ")" << endl
			<< "pos1  = " << pos1 << " (negated: " << -pos1 << ")" << endl
			<< "neg10 = " << neg10 << " (negated: " << -neg10 << ")" << endl
			<< "pos10 = " << pos10 << " (negated: " << -pos10 << ")" << endl;
		if( !neg1.isNegative() || pos1.isNegative() )
			throw logic_error("Positive numbers are !isNegative(), negative numbers are isNegative()" );

		if( neg1 > 0 )
			throw logic_error("Negative numbers should be less than zero");
		if( pos1 < 0 )
			throw logic_error("Positive numbers should be greater than zero");

		if( (neg1 * neg1).isNegative() )
			throw logic_error("Negative times negative should be positive");
		if( !(neg1 * pos1).isNegative() )
			throw logic_error("Negative times positive should be positive");
		if( !(pos1 * neg1).isNegative() )
			throw logic_error("Positive times negative should be positive");

		log() << "neg1 + pos1 = " << (neg1 + pos1) << endl;
		log() << "pos1 + neg1 = " << (pos1 + neg1) << endl;
		log() << "neg1 - pos1 = " << (neg1 - pos1) << endl;
		log() << "pos1 - neg1 = " << (pos1 - neg1) << endl;

		if( neg1 + pos1 != 0 )
			throw logic_error("Negative plus positive should cancel");
		if( pos1 + neg1 != 0 )
			throw logic_error("Positive plus negative should cancel");
		if( neg1 - pos1 != -2 )
			throw logic_error("Negative minus positive should be negative");
		if( pos1 - neg1 != 2 )
			throw logic_error("Positive minus negative should be positive");
		if( neg1 + neg1 != -2 )
			throw logic_error("Negative plus positive should be negative");
		if( neg1 - neg1 != 0 )
			throw logic_error("Negative minus positive should cancel");

		log() << "neg1 + pos10 = " << (neg1 + pos10) << endl;
		log() << "pos1 + neg10 = " << (pos1 + neg10) << endl;
		log() << "neg1 - pos10 = " << (neg1 - pos10) << endl;
		log() << "pos1 - neg10 = " << (pos1 - neg10) << endl;

		if( neg1 + pos10 != 9 )
			throw logic_error("Small negative plus large positive should be positive");
		if( pos1 + neg10 != -9 )
			throw logic_error("Small positive plus large negative should be negative");
		if( neg1 - pos10 != -11 )
			throw logic_error("Small negative minus large positive should be negative");
		if( pos1 - neg10 != 11 )
			throw logic_error("Small positive minus large negative should be positive");

		if( neg1.abs() != pos1 )
			throw logic_error("Absolute part of a negative number should be positive" );

		log() << "neg1 * pos1 = " << (neg1 * pos1) << endl;
		log() << "neg1 * neg1 = " << (neg1 * neg1) << endl;
		log() << "pos1 * pos1 = " << (pos1 * neg1) << endl;
		log() << "pos1 * pos1 = " << (pos1 * pos1) << endl;

		if( neg1 * pos1 != -1 )
			throw logic_error("Negative times positive should be negative");
		if( neg1 * neg1 != 1 )
			throw logic_error("Negative times negative should be positive");
		if( pos1 * neg1 != -1 )
			throw logic_error("Positive times negative should be negative");
		if( pos1 * pos1 != 1 )
			throw logic_error("Positive times positive should be positive");

		log() << "neg1 / pos1 = " << (neg1 / pos1) << endl;
		log() << "neg1 / neg1 = " << (neg1 / neg1) << endl;
		log() << "pos1 / pos1 = " << (pos1 / neg1) << endl;
		log() << "pos1 / pos1 = " << (pos1 / pos1) << endl;

		if( neg1 / pos1 != -1 )
			throw logic_error("Negative divided by positive should be negative");
		if( neg1 / neg1 != 1 )
			throw logic_error("Negative divided by negative should be positive");
		if( pos1 / neg1 != -1 )
			throw logic_error("Positive divided by negative should be negative");
		if( pos1 / pos1 != 1 )
			throw logic_error("Positive divided by positive should be positive");

		log() << "neg1 % pos10 = " << (neg1 % pos10) << endl;
		log() << "neg1 % neg10 = " << (neg1 % neg10) << endl;
		log() << "pos1 % pos10 = " << (pos1 % neg10) << endl;
		log() << "pos1 % pos10 = " << (pos1 % pos10) << endl;

		if( neg1 % pos10 != -1 )
			throw logic_error("Negative mod positive should be negative");
		if( neg1 % neg10 != -1 )
			throw logic_error("Negative mod negative should be negative");
		if( pos1 % neg10 != 1 )
			throw logic_error("Positive mod negative should be positive");
		if( pos1 % pos10 != 1 )
			throw logic_error("Positive mod positive should be positive");

	} catch( exception &e ) {
		log(TLog::Error) << e.what() << endl;
		return 255;
	}

	try {
		struct {
			string Sample;
			unsigned int Base;
		} TestCases[] = {
			{ string("56bc75e2d630fffff"), 16 },
			{ string("99999999999999999999"), 10 },
			{ string(), 0 }
		}, *p = TestCases;

		log(TLog::Status) << "Testing display" << endl;

		while( p->Base != 0 ) {
			TBigInteger test( p->Sample, p->Base );
			string x;

			if( p->Base == 16 ) {
				ostringstream oss;
				oss << hex << test << dec;
				x = oss.str();
			} else {
				x = test.toString(p->Base);
			}
			log() << "\"" << p->Sample << "\" in base" << p->Base << " = " << x << endl;
			if( x != p->Sample )
				throw logic_error("Displaying in custom base failed");

			p++;
		}

		log(TLog::Status) << "Testing successive multiplication (in base 64)" << endl;
		TBigInteger j = 1;
		for( unsigned b = 0; b < 20; b++ ) {
			j *= 64;
		}
		log() << "j_64 = " << j.toString(64) << " is 0x" << hex << j << dec << endl;
		if( j.toString(64) != "BAAAAAAAAAAAAAAAAAAAA===" )
			throw logic_error( "1*64^x should always begin with B in base64" );

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


// ----------------------------------------------------------------------------
// Project: library
/// @file   hashtypes.h
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
#ifndef HASHTYPES_H
#define HASHTYPES_H

// -------------- Includes
// --- C
// --- C++
// --- Qt
// --- OS
// --- Project lib
#include "extraint.h"
// --- Project


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

class TBitcoinBase58 : public TBigUnsignedInteger
{
  public:
	TBitcoinBase58( const TBitcoinBase58 &O ) { operator=(O); }
	TBitcoinBase58( const string &s, unsigned int b = 58 ) { fromString(s,b); }

	ostream &printOn( ostream &s ) const;

	string toString() const { return TBigUnsignedInteger::toString(58); }

  protected:
	unsigned int fromCharacter( unsigned int, unsigned int ) const;
	unsigned int toCharacter( unsigned int, unsigned int ) const;
};

//
// Class:	TBitcoinHash
// Description:
//
class TBitcoinHash : public TBigUnsignedInteger
{
  public:
	TBitcoinHash() { invalidate(); }
	TBitcoinHash( const TBigUnsignedInteger &O ) { operator=(O); }
	TBitcoinHash( const TBitcoinHash &O ) { operator=(O); }
	TBitcoinHash( const string &s ) { fromString(s,16); }
	TBitcoinHash( int t ) : TBigUnsignedInteger(t) {}

	ostream &printOn( ostream &s ) const;

	TBitcoinHash reversedBytes() const;

	// Import operators masked by C++ defaults
	using TBigUnsignedInteger::operator=;

  protected:
	string stringPad( const string &, unsigned int ) const;

	static const unsigned int HASH_BYTES;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif

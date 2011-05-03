// ----------------------------------------------------------------------------
// Project: library
/// @file   base58.h
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
#ifndef BASE58_H
#define BASE58_H

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

class TBitcoinBase58 : public TBigInteger
{
  public:
	TBitcoinBase58( const TBitcoinBase58 &O ) { operator=(O); }
	TBitcoinBase58( const string &s, unsigned int b = 58 ) { fromString(s,b); }

	ostream &printOn( ostream &s ) const;

  protected:
	unsigned int fromCharacter( unsigned int, unsigned int ) const;
	unsigned int toCharacter( unsigned int, unsigned int ) const;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif

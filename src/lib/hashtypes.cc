// ----------------------------------------------------------------------------
// Project: library
/// @file   base58.cc
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
#include "hashtypes.h"

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <iostream>
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class member definitions

//
// Function:	TBitcoinBase58 :: fromCharacter
// Description:
//
unsigned int TBitcoinBase58::fromCharacter( unsigned int ch, unsigned int Base ) const
{
	static const uint8_t ASCIIToBase58[] = {
		// Output (0-57)                                 // Input (ASCII)
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,  // + . . . / 0 1 2
		0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xff,  // 3 4 5 6 7 8 9 .
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x09, 0x0a,  // . . . . . . A B
		0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0xff, 0x11,  // C D E F G H I J
		0x12, 0x13, 0x14, 0x15, 0xff, 0x16, 0x17, 0x18,  // K L M N O P Q R
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,  // S T U V W X Y Z
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x21, 0x22,  // . . . . . . a b
		0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,  // c d e f g h i j
		0x2b, 0xff, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,  // k l m n o p q r
		0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39   // s t u v w x y z
	};
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base != 58 )
		return TBigUnsignedInteger::fromCharacter( ch, Base );

	if( ch >= '+' && ch-'+' <= sizeof(ASCIIToBase58) ) {
		ch = ASCIIToBase58[ch-'+'];
		if( ch == 0xff )
			ch = INVALID;
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TBitcoinBase58 :: toCharacter
// Description:
//
unsigned int TBitcoinBase58::toCharacter( unsigned int ch, unsigned int Base ) const
{
	static const char Base58ToASCII[59] =
		"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base != 58 )
		return TBigUnsignedInteger::toCharacter( ch, Base );

	if( ch <= sizeof(Base58ToASCII) ) {
		ch = Base58ToASCII[ch];
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TBitcoinBase58 :: printOn
// Description:
//
ostream &TBitcoinBase58::printOn( ostream &s ) const
{
	if( !isValid() ) {
		s << "!INVALID!";
		return s;
	}

	if( s.flags() & ostream::hex ) {
		TBigUnsignedInteger::printOn(s);
	} else {
		s << toString();
	}

	return s;
}

// ------

//
// Static:	TBitcoinHash :: HASH_BYTES
// Description:
//
const unsigned int TBitcoinHash::HASH_BYTES = 32;

//
// Function:	TBitcoinHash :: stringPad
// Description:
//
string TBitcoinHash::stringPad( const string &s, unsigned int Base ) const
{
	if( s.size() < HASH_BYTES*2 ) {
		string pad;
		pad.assign( HASH_BYTES*2 - s.size(), toCharacter(0,Base) );
		return pad + s;
	} else if( s.size() > HASH_BYTES*2 ) {
		throw logic_error( "TBitcoinHashes can't be more than HASH_BYTES bytes" );
	} else {
		return s;
	}
}

//
// Function:	TBitcoinHash :: printOn
// Description:
//
ostream &TBitcoinHash::printOn( ostream &s ) const
{
	if( !isValid() ) {
		s << "!INVALID!";
		return s;
	}

	// Force the use of string pad by calling it even for hex (which the
	// base class doesn't do)
	string x = toString(16);
	s << x.substr(0,20) << "..." << x.substr(x.size()-4, x.size()-1);

	return s;
}

//
// Function:	TBitcoinHash :: reversedBytes
// Description:
//
TBitcoinHash TBitcoinHash::reversedBytes() const
{
	tIndex OldHighestBit = highestBit();
	TBitcoinHash X( TBigUnsignedInteger::reversedBytes() );

	// Consider these blocks  (highest bit 75)
	//   XXXX0908 07060504 03020100
	// After base-class reversal these will be
	//   00000001 02030405 06070809
	// Lets say we want a 18 byte hash, then our left shift will be
	// (18-(75/8))*8 = 9
	//   0102 03040506 07080900 00000000 00000000
	// Final size 18 bytes, flipped version of original.

	// The bytes are now reversed, but that's not enough, we have to
	// reverse to a fixed width for a hash.  For that we need to know
	// how many bytes we were short of 32 originally.
	X <<= (HASH_BYTES - 1 - (OldHighestBit/8)) * 8;

	return X;
}

// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <stdexcept>

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		cerr << "Testing constructors and initialisation" << endl;

		TBitcoinBase58 i("1Eym7pyJcaambv8FG4ZoU8A4xsiL9us2zz");
		TBitcoinBase58 j("1111111111111111111114oLvT2"); // 0x94a00911
		// Bitcoin addresses are 1 version byte, 20 bytes of hashed
		// ECDSA key followed by four bytes of checksum (which is itself
		// a hash of the hashed key and version bytes).  The largest
		// possible address is therefore 25 bytes of 0xff
		TBitcoinBase58 k("ffffffffffffffffffffffffffffffffffffffffffffffffff", 16);
		// The above gives a 34 character base58 string.  What's the
		// biggest 34 digit base58 number though?
		TBitcoinBase58 l("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");

		cerr << "i = " << i << "; 0x" << hex << i << dec << endl;
//		if( i.getBlock(2) != 0x5 || i.getBlock(1) != 0x6bc75e2d || i.getBlock(0) != 0x630fffff )
//			throw logic_error("Assignment from decimal-representing string incorrect");
		cerr << "j = " << j << "; 0x" << hex << j << dec << endl;
		cerr << "k = " << k << "; 0x" << hex << k << dec << endl;
		k++;
		cerr << "k = " << k << "; 0x" << hex << k << dec << endl;
		cerr << "l = " << l << "; 0x" << hex << l << dec << endl;
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	try {
		cerr << "Testing constructors and initialisation" << endl;

		TBitcoinHash i("0"); // 0x94a00911
		TBitcoinHash j("ffffffffffffffffffffffffffffffffffffffffffffffffff");

		cerr << "i = " << i << endl;
		cerr << "j = " << j << endl;
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

